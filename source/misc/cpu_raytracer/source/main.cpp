
// [SDL]
#pragma region sdl
    #if _DEBUG
#pragma comment (lib, "SDL2d.lib")
    #else
#pragma comment (lib, "SDL2.lib")
    #endif
    #include "SDL/SDL.h"
    #ifdef main
    #undef main
    #endif
#pragma endregion

// [INFLUX]
#pragma region core
    #include "core/basetypes.h"
    #include "core/math/vector.h"
    #include "core/time.h"
    #include "core/math/random.h"
#pragma endregion

// [STL]
#pragma region stl
    #include <iostream>
    #include <array>
#pragma endregion

// [PIX]
#include "PixEvents.h"

// ================================================
// basic types
using uint8 = unsigned char;

// ================================================
// constants
constexpr size_t    gWindowWidth = 640;
constexpr size_t    gWindowHeight = 480;
constexpr size_t    gNumPixels = gWindowWidth * gWindowHeight;
constexpr float     gAspectRatio = static_cast<float>(gWindowWidth) / static_cast<float>(gWindowHeight);
constexpr uint32_t  gNumFramesPerLog = 10;
constexpr uint32_t  gNumFramesPerAverage = 60;

// Setup scene:
constexpr uint32_t  gNumSpheres = 1;
constexpr float     gSpheresMin = 0.0f;
constexpr float     gSpheresMax = 0.0f;
constexpr float     gSpheresMinSize = 1.0f;
constexpr float     gSpheresMaxSize = 1.0f;
constexpr float     gSpheresMinDepth = 80.0f;
constexpr float     gSpheresMaxDepth = 100.0f;

// ================================================
// sub-headers
#include "PixelRaytracer.h"
#include "bvh.h"

#define MULTITHREADED 0
#if MULTITHREADED
constexpr uint8_t gNumThreads = 8u;
const size_t gThreadRange = (size_t)std::ceil(static_cast<double>(gNumPixels) / static_cast<double>(gNumThreads));
#endif

#define STATS 0

struct timing final
{
    float m_delta_time;
    float m_time;
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
    void add(const double value)
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

void update_scene(const timing& sceneTime, influx::render_scene& scene)
{
    float pingPong{};

    for (size_t i = 0; i < scene.m_spheres.size(); ++i)
    {
        influx::math::sphere<float>& sphere = scene.m_spheres[i];
        pingPong = influx::math::pingpong(sceneTime.m_time + scene.m_randoms[i], 1.0f);
        sphere.m_radius = influx::math::lerp(pingPong, gSpheresMinSize, gSpheresMaxSize);

        sphere.m_position.x = influx::math::cos(pingPong) * gSpheresMax * scene.m_randoms[i];
        sphere.m_position.y = influx::math::sin(pingPong) * gSpheresMax * scene.m_randoms[i];
        sphere.m_position.z = influx::math::lerp(pingPong, gSpheresMinDepth, gSpheresMaxDepth);
    }
}

int main()
{
    using namespace influx::math;

#if 0
    static constexpr unsigned int k_max_num_primitives = 1024u;
    using bvh = influx::bvh_3D<k_max_num_primitives, 10u>;
    bvh the_bvh;
    bvh::ray the_ray;

    using primitive = bvh::sphere;
    std::vector<primitive> all_spheres(k_max_num_primitives);
    the_bvh.rebuild(all_spheres);
    the_bvh.test_hit(the_ray, all_spheres);
#endif

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

    influx::random::seed_random();

    // Setup Renderscane:
    influx::render_scene scene{};
    {
        scene.m_spheres = influx::random::get_random_spherefs<gNumSpheres>(
            { gSpheresMin, gSpheresMax },
            { gSpheresMinSize, gSpheresMaxSize });

        scene.m_randoms = influx::random::get_randoms<float, gNumSpheres>(0.0f, 1.0f);
        scene.m_light.m_colour      = float3::make_one();
        scene.m_light.m_direction   = float3{ 0.33f, -0.33f, -0.33f }.normalized();
    }

    // Setup Renderer & Camera:
    influx::pixel_raytracer renderer = influx::pixel_raytracer();
    renderer.set_camera_fov(90.0f);
    renderer.set_camera_position({0.0f, 0.0f, -10.0f });
    renderer.set_camera_forward({ 0.0f, 0.0f, 1.0f });
    renderer.get_render_settings().m_depth_min_max.y = 500.0f;

#if MULTITHREADED
    influx::threadpool<gNumThreads>* renderJobPool = new influx::threadpool<gNumThreads>();
#else
    float uvx{};
    float uvy{};
#endif

    // Setup timers:
    influx::time::point beforeFrame = influx::time::get_now();
    influx::time::point beforeUpdate = influx::time::get_now();
    influx::time::point beforeRender = influx::time::get_now();
    influx::time::point beforePresent = influx::time::get_now();

    uint64_t currentFrame{};
    timing sceneTime{};
    Stats stats{};
    bool isQuit = false;

    while (!isQuit)
    {
        beforeFrame = influx::time::get_now();

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
#if STATS
        stats.add<Stats::EStat::Update>(influx::time::measure_ms<double>([&sceneTime, &scene]()
        {
            update_scene(sceneTime, scene);
        }));
#endif

        // ¬ RENDER
        beforeRender = influx::time::get_now();
#if MULTITHREADED
        for (size_t i = 0, jobOffset = 0; i < gNumThreads; ++i)
        {
            jobOffset = i * gThreadRange;

            renderJobPool->QueueJob([i, jobOffset, &backbufferPixels, &renderer, &scene, &depthBufferPixels, &averageScreenDepth]()
            {
                for (size_t p = jobOffset; p < jobOffset + gThreadRange; ++p)
                {
                    float uvx = float(p % gWindowWidth) / float(gWindowWidth);
                    float uvy = float(p / gWindowWidth) / float(gWindowHeight);

                    influx::pixel_renderer::pixel_output pixelResult =
                        renderer.render_pixel(scene, { uvx, uvy }, gAspectRatio);

                    const size_t pixelBaseIdx = p * 4u;

                    backbufferPixels[pixelBaseIdx]      = static_cast<uint8>(255.0f * pixelResult.m_rgba.b); // B
                    backbufferPixels[pixelBaseIdx + 1u] = static_cast<uint8>(255.0f * pixelResult.m_rgba.g); // G
                    backbufferPixels[pixelBaseIdx + 2u] = static_cast<uint8>(255.0f * pixelResult.m_rgba.r); // R
                    backbufferPixels[pixelBaseIdx + 3u] = static_cast<uint8>(255.0f * pixelResult.m_rgba.a); // A

                    depthBufferPixels[p] = pixelResult.m_depth;
                }
            });
        }
        renderJobPool->WaitUntilFinished();

#else
        for (size_t i = 0; i < gNumPixels; ++i)
        {
            uvx = float(i % gWindowWidth) / float(gWindowWidth);
            uvy = float(i / gWindowWidth) / float(gWindowHeight);

            auto pxResult = renderer.render_pixel(scene, { uvx, uvy }, gAspectRatio);

            const size_t pixelBaseIdx = i * 4u;

            backbufferPixels[pixelBaseIdx]      = static_cast<uint8>(255.0f * pxResult.m_rgba.b); // B
            backbufferPixels[pixelBaseIdx + 1u] = static_cast<uint8>(255.0f * pxResult.m_rgba.g); // G
            backbufferPixels[pixelBaseIdx + 2u] = static_cast<uint8>(255.0f * pxResult.m_rgba.r); // R
            backbufferPixels[pixelBaseIdx + 3u] = static_cast<uint8>(255.0f * pxResult.m_rgba.a); // A

            depthBufferPixels[i] = pxResult.m_depth;
        }
#endif
#if STATS
        stats.add<Stats::EStat::Render>(influx::time::get_ms_between<double>(influx::time::get_now(), beforeRender));
#endif
        // ¬ PRESENT
        beforePresent = influx::time::get_now();
        SDL_BlitSurface(backbuffer_surface, NULL, window_surface, NULL);
        SDL_UpdateWindowSurface(window);
#if STATS
        stats.add<Stats::EStat::Present>(influx::time::get_ms_between<double>(influx::time::get_now(), beforePresent));
#endif
        // ¬ COMPILE FRAMETIME
        double thisFrame = influx::time::get_ms_between<double>(influx::time::get_now(), beforeFrame);
#if STATS
        stats.add<Stats::EStat::Frame>(thisFrame);
#endif
        sceneTime.m_delta_time = static_cast<float>(thisFrame / 1000);
        sceneTime.m_time += sceneTime.m_delta_time;

        // ¬ LOG
#if STATS
        if (currentFrame > 0 && currentFrame % gNumFramesPerLog == 0)
        {
            std::cout << "\x1B[2J\x1B[H";

            double msFrame = stats.GetValue<Stats::EStat::Frame>();
            double fps = (1 / msFrame * 1000);
            
            std::cout << "-- FPS: " << fps << " \n";
            using value_and_index = std::pair<double, size_t>;
            std::array<value_and_index, Stats::k_EnumSize> sortedStats{};
            for (size_t i = 0; i < Stats::k_EnumSize; ++i)
            {
                sortedStats[i] = { stats.Values[i], i };
            }
            std::sort(sortedStats.begin(), sortedStats.end(), [](const value_and_index& a, const value_and_index& b) -> bool
                {
                    bool aIsBigger = a.first > b.first;
                    if (aIsBigger && ((Stats::EStat)b.second == Stats::EStat::Frame)) return !aIsBigger;
                    return aIsBigger;
                });

            for (size_t i = 0; i < Stats::k_EnumSize; ++i)
            {
                
                using namespace influx::platform;
                double value = sortedStats[i].first;
                if (value > 16.0) set_console_colour_attribute(e_console_colour::red);
                else set_console_colour_attribute(e_console_colour::green);

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
#endif
        ++currentFrame;
    }

#if MULTITHREADED
    delete renderJobPool;
    renderJobPool = nullptr;
#endif

    delete[] depthBufferPixels;

    SDL_FreeSurface(backbuffer_surface);
}