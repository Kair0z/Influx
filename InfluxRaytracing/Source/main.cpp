
#include "PixEvents.h"

#if _DEBUG
#pragma comment (lib, "SDL2d.lib")
#else
#pragma comment (lib, "SDL2.lib")
#endif

#include "SDL/SDL.h"
#include <iostream>
#include <array>

#ifdef main
#undef main
#endif

#include "Core/Math/Vector.h"
#include "Core/Time.h"
#include "Core/Threading/ThreadPool.h"
#include "Core/Math/Random.h"
#include "Core/KDTree.h"
#include "Core/Platform/WindowsPlatform.h"

using uint8 = unsigned char;

constexpr size_t  gWindowWidth = 640;
constexpr size_t  gWindowHeight = 480;
constexpr size_t  gNumPixels = gWindowWidth * gWindowHeight;
constexpr float     gAspectRatio = static_cast<float>(gWindowWidth) / static_cast<float>(gWindowHeight);
constexpr uint32_t  gNumFramesPerLog = 10;
constexpr uint32_t  gNumFramesPerAverage = 60;

// Setup scene:
constexpr uint32_t gNumSpheres = 25;
constexpr float gSpheresMin = -100.0f;
constexpr float gSpheresMax = 100.0f;
constexpr float gSpheresMinSize = 5.0f;
constexpr float gSpheresMaxSize = 10.0f;
constexpr float gSpheresMinDepth = 80.0f;
constexpr float gSpheresMaxDepth = 100.0f;

#define THREADED_RENDERING 1
#if THREADED_RENDERING
constexpr uint8_t gNumThreads = 8u;
const size_t gThreadRange = (size_t)std::ceil(static_cast<double>(gNumPixels) / static_cast<double>(gNumThreads));
#endif

#include "PixelRaytracer.h"

struct Time final
{
    float DeltaTime;
    float Time;
};

struct Stats final
{
    enum class EStat
    {
        Render,
        Update,
        Present,
        Frame,
        Max
    };

    constexpr static size_t k_EnumSize = static_cast<size_t>(EStat::Max);
    static constexpr char const* k_StatToNames[k_EnumSize]
    {
        "Render",
        "Update",
        "Present",
        "Frame",
    };

    
    std::array<double, k_EnumSize> Values{};
    std::array<double, k_EnumSize> ValueSums{};
    std::array<size_t, k_EnumSize> AverageCounter{};

    template <EStat _S>
    void AddValue(const double value)
    {
        constexpr size_t idx = static_cast<size_t>(_S);
        ++AverageCounter[idx];
        Values[idx] = value;
        ValueSums[idx] += value;
    }

    template <EStat _S>
    double GetValue() const
    {
        constexpr size_t idx = static_cast<size_t>(_S);
        return Values[idx];
    }

    template <EStat _S>
    double GetAverage() const
    {
        constexpr size_t idx = static_cast<size_t>(_S);
        size_t averageCounter = AverageCounter[idx];

        if (averageCounter == 0u) return 0.0;
        return ValueSums[idx] / averageCounter;
    }

    void Reset()
    {
        Values = {};
        ValueSums = {};
        AverageCounter = {};
    }
};

void UpdateScene(const Time& sceneTime, Influx::RenderScene& scene)
{
    float pingPong{};

    for (size_t i = 0; i < scene.Spheres.size(); ++i)
    {
        Influx::Math::Sphere<float>& sphere = scene.Spheres[i];
        pingPong = Influx::Math::PingPong(sceneTime.Time + scene.Randoms[i], 1.0f);
        sphere.m_radius = Influx::Math::Lerp(pingPong, gSpheresMinSize, gSpheresMaxSize);

        sphere.m_position.x = Influx::Math::Cos(pingPong) * gSpheresMax * scene.Randoms[i];
        sphere.m_position.y = Influx::Math::Sin(pingPong) * gSpheresMax * scene.Randoms[i];
        sphere.m_position.z = Influx::Math::Lerp(pingPong, gSpheresMinDepth, gSpheresMaxDepth);
    }
}

int main()
{
    using namespace Influx::Math;

#pragma region Setup
    int a = 0;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        // Oops
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Raytracer \n",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        gWindowWidth, gWindowHeight, 0);

    if (!window)
    {
        // Another oops
        return -1;
    }

    SDL_Surface* window_surface = SDL_GetWindowSurface(window);
    if (!window_surface)
    {
        return -1;
    }

    // Create a backbuffer to blit into our window...
    SDL_Surface* backbuffer_surface = SDL_DuplicateSurface(window_surface);
    float* depthBufferPixels = new float[gNumPixels]{};
    float averageScreenDepth = 0.0f;
    unsigned char* backbufferPixels = (unsigned char*)backbuffer_surface->pixels;
#pragma endregion

    // Seed our random:
    Influx::Random::SeedRandom();


    // Setup Renderscane:
    Influx::RenderScene scene{};
    scene.Spheres = Influx::Random::Sphere::RandomSpherefs<gNumSpheres>({gSpheresMin, gSpheresMin, gSpheresMinDepth}, {gSpheresMax, gSpheresMax, gSpheresMaxDepth}, {gSpheresMinSize, gSpheresMaxSize});
    scene.Randoms = Influx::Random::Randoms<float, gNumSpheres>(0.0f, 1.0f);
    scene.MainLight.Colour = Vectorf3::One();
    scene.MainLight.Direction = { 0.33f, -0.33f, -0.33f };
    scene.MainLight.Direction.Normalize();


    // Setup Renderer & Camera:
    Influx::PixelRaytracer renderer = Influx::PixelRaytracer();

    renderer.SetCameraFieldOfView(90.0f);
    renderer.SetCameraPosition({});
    renderer.SetCameraForward({ 0.0f, 0.0f, 1.0f });
    renderer.GetRenderSettings().RenderDepthMinMax.y = 500.0f;

#if THREADED_RENDERING
    Influx::ThreadPool<gNumThreads>* renderJobPool = new Influx::ThreadPool<gNumThreads>();
#else
    float uvx{};
    float uvy{};
#endif

    // Setup timers:
    Influx::Time::TimePoint beforeFrame = Influx::Time::Now();
    Influx::Time::TimePoint beforeUpdate = Influx::Time::Now();
    Influx::Time::TimePoint beforeRender = Influx::Time::Now();
    Influx::Time::TimePoint beforePresent = Influx::Time::Now();

    uint64_t currentFrame{};
    Time sceneTime{};
    Stats stats{};
    bool isQuit = false;

    while (!isQuit)
    {
        beforeFrame = Influx::Time::Now();

        // ¬ POLL SDL WINDOW EVENTS
        SDL_Event e;
        while (SDL_PollEvent(&e) > 0)
        {
            switch (e.type)
            {
            case SDL_QUIT:
                isQuit = true;
                break;
            }
        }

        // ¬ UPDATE:
        beforeUpdate = Influx::Time::Now();
        UpdateScene(sceneTime, scene);
        stats.AddValue<Stats::EStat::Update>(Influx::Time::MsBetween<double>(Influx::Time::Now(), beforeUpdate));
        
        // ¬ RENDER
        beforeRender = Influx::Time::Now();
#if THREADED_RENDERING
        for (size_t i = 0, jobOffset = 0; i < gNumThreads; ++i)
        {
            jobOffset = i * gThreadRange;

            renderJobPool->QueueJob([i, jobOffset, &backbufferPixels, &renderer, &scene, &depthBufferPixels, &averageScreenDepth]()
                {
                    for (size_t p = jobOffset; p < jobOffset + gThreadRange; ++p)
                    {
                        float uvx = float(p % gWindowWidth) / float(gWindowWidth);
                        float uvy = float(p / gWindowWidth) / float(gWindowHeight);

                        Influx::PixelRenderer::PixelOutput pixelResult =
                            renderer.RenderPixel(scene, { uvx, uvy }, gAspectRatio);

                        const size_t pixelBaseIdx = p * 4u;

                        backbufferPixels[pixelBaseIdx]      = static_cast<uint8>(255.0f * pixelResult.RGBA.b); // B
                        backbufferPixels[pixelBaseIdx + 1u] = static_cast<uint8>(255.0f * pixelResult.RGBA.g); // G
                        backbufferPixels[pixelBaseIdx + 2u] = static_cast<uint8>(255.0f * pixelResult.RGBA.r); // R
                        backbufferPixels[pixelBaseIdx + 3u] = static_cast<uint8>(255.0f * pixelResult.RGBA.a); // A

                        depthBufferPixels[p] = pixelResult.Depth;
                    }
                });
        }
        renderJobPool->WaitUntilFinished();

#else
        for (size_t i = 0; i < gNumPixels; ++i)
        {
            uvx = float(i % gWindowWidth) / float(gWindowWidth);
            uvy = float(i / gWindowWidth) / float(gWindowHeight);

            auto pxResult = raytracer.RenderPixel(scene, { uvx, uvy }, gAspectRatio);

            const size_t pixelBaseIdx = i * 4u;

            backbufferPixels[pixelBaseIdx]      = 255.0f * pxResult.RGBA.b; // B
            backbufferPixels[pixelBaseIdx + 1u] = 255.0f * pxResult.RGBA.g; // G
            backbufferPixels[pixelBaseIdx + 2u] = 255.0f * pxResult.RGBA.r; // R
            backbufferPixels[pixelBaseIdx + 3u] = 255.0f * pxResult.RGBA.a; // A

            depthBufferPixels[i] = pxResult.Depth;
        }
#endif
        stats.AddValue<Stats::EStat::Render>(Influx::Time::MsBetween<double>(Influx::Time::Now(), beforeRender));

        // ¬ PRESENT
        beforePresent = Influx::Time::Now();
        SDL_BlitSurface(backbuffer_surface, NULL, window_surface, NULL);
        SDL_UpdateWindowSurface(window);
        stats.AddValue<Stats::EStat::Present>(Influx::Time::MsBetween<double>(Influx::Time::Now(), beforePresent));
        
        // ¬ COMPILE FRAMETIME
        double thisFrame = Influx::Time::MsBetween<double>(Influx::Time::Now(), beforeFrame);
        stats.AddValue<Stats::EStat::Frame>(thisFrame);
        sceneTime.DeltaTime = static_cast<float>(thisFrame / 1000);
        sceneTime.Time += sceneTime.DeltaTime;
        
        // ¬ LOG
        if (currentFrame > 0 && currentFrame % gNumFramesPerLog == 0)
        {
            std::cout << "\x1B[2J\x1B[H";

            double msFrame = stats.GetValue<Stats::EStat::Frame>();
            double fps = (1 / msFrame * 1000);
            
            std::cout << "-- FPS: " << fps << " \n";
            using ValueAndIndex = std::pair<double, size_t>;
            std::array<ValueAndIndex, Stats::k_EnumSize> sortedStats{};
            for (size_t i = 0; i < Stats::k_EnumSize; ++i)
            {
                sortedStats[i] = { stats.Values[i], i };
            }
            std::sort(sortedStats.begin(), sortedStats.end(), [](const ValueAndIndex& a, const ValueAndIndex& b) -> bool
                {
                    bool aIsBigger = a.first > b.first;
                    if (aIsBigger && ((Stats::EStat)b.second == Stats::EStat::Frame)) return !aIsBigger;
                    return aIsBigger;
                });

            for (size_t i = 0; i < Stats::k_EnumSize; ++i)
            {
                using namespace Influx::Platform;
                double value = sortedStats[i].first;
                if (value > 16.0) SetConsoleColourAttribute<EConsoleColour::Red>();
                else SetConsoleColourAttribute<EConsoleColour::Green>();

                std::cout << "Ms " << Stats::k_StatToNames[sortedStats[i].second] << ": " << value << "\n";
            }

            double avg_msFrame = stats.GetAverage<Stats::EStat::Frame>();
            double avg_fps = (1 / avg_msFrame * 1000);
            
            std::cout << "\n-- Avg FPS: " << avg_fps << " \n";
            std::cout << "Avg Ms Total: " << avg_msFrame << " \n";
        }
        
        // ¬ Reset stats & counters
        if (currentFrame > 0 && currentFrame % gNumFramesPerAverage == 0)
            stats.Reset();
        
        ++currentFrame;
    }

#if THREADED_RENDERING
    delete renderJobPool;
    renderJobPool = nullptr;
#endif

    delete[] depthBufferPixels;

    SDL_FreeSurface(backbuffer_surface);
}