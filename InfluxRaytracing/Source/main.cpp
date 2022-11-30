
#include "PixEvents.h"

#if _DEBUG
#pragma comment (lib, "SDL2d.lib")
#else
#pragma comment (lib, "SDL2.lib")
#endif

#include "SDL/SDL.h"
#include <iostream>

#ifdef main
#undef main
#endif

#include "Core/Math/Vector.h"
#include "Core/Time.h"
#include "Core/Procedure/ThreadPool.h"
#include "Core/Math/Random.h"
#include "Core/KDTree.h"

constexpr size_t  gWindowWidth = 1920;
constexpr size_t  gWindowHeight = 1080;
constexpr size_t  gNumPixels = gWindowWidth * gWindowHeight;
constexpr float     gAspectRatio = static_cast<float>(gWindowWidth) / static_cast<float>(gWindowHeight);
constexpr uint32_t  gNumFramesPerLog = 60;

#define THREADED_RENDERING 1
#if THREADED_RENDERING
constexpr uint8_t gNumThreads = 8;
const size_t gThreadRange = (size_t)std::ceil(static_cast<double>(gNumPixels) / static_cast<double>(gNumThreads));
#endif

#include "PixelRaytracer.h"

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
    unsigned char* backbufferPixels = (unsigned char*)backbuffer_surface->pixels;
#pragma endregion

    double msRender = 0.0f;
    double msPresent = 0.0f;
    double sumFPS = 0.0f;
    uint64_t currentFrame{};

    Influx::Time::TimePoint beforeRender = Influx::Time::Now();
    Influx::Time::TimePoint beforePresent = Influx::Time::Now();

    Influx::Random::SeedRandom();

    float min = -50.0f;
    float max = 50.0f;
    float minSize = 10.0f;
    float maxSize = 25.0f;
    float depth = 100.0f;

    std::vector<Influx::Math::Sphere<float>> scene_spheres
    {
        Influx::Random::Sphere::RandomSpherefs<1200>({min, min, depth}, {max, max, depth}, {minSize, maxSize})
    };

    Influx::KDTree<3u> tree = Influx::KDTree<3u>(
        Influx::Random::Vector::Random3fs<23u>( Vectorf3{1.0f, 1.0f, 1.0f}, Vectorf3{2.0f, 2.0f, 2.0f} ));

    tree.Build();

    Influx::PixelRaytracer raytracer = Influx::PixelRaytracer();
    raytracer.SetCameraFieldOfView(90.0f);
    raytracer.SetCameraPosition({});
    raytracer.SetCameraForward({ 0.0f, 0.0f, 1.0f });

#if THREADED_RENDERING
    Influx::ThreadPool<gNumThreads>* renderJobPool = new Influx::ThreadPool<gNumThreads>();
#else
    float uvx{};
    float uvy{};
    Influx::PixelRenderer::PixelColour pxColour{};
#endif

    bool isQuit = false;
    while (!isQuit)
    {
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

        // RENDER
        beforeRender = Influx::Time::Now();

        {
#ifdef PROFILE
            PIXScopedEvent(0, "Raytrace Frame");
#endif
#if THREADED_RENDERING
            for (size_t i = 0, jobOffset = 0; i < gNumThreads; ++i)
            {
                jobOffset = i * gThreadRange;

                renderJobPool->QueueJob([i, jobOffset, &backbufferPixels, &raytracer, &scene_spheres]()
                    {
                        for (size_t p = jobOffset; p < jobOffset + gThreadRange; ++p)
                        {
                            float uvx = float(p % gWindowWidth) / float(gWindowWidth);
                            float uvy = float(p / gWindowWidth) / float(gWindowHeight);

                            Influx::PixelRenderer::PixelColour pxColour =
                                raytracer.RenderPixel(scene_spheres, { uvx, uvy }, gAspectRatio);

                            const size_t pixelBaseIdx = p * 4u;

                            backbufferPixels[pixelBaseIdx] = pxColour.b; // B
                            backbufferPixels[pixelBaseIdx + 1u] = pxColour.g; // G
                            backbufferPixels[pixelBaseIdx + 2u] = pxColour.r; // R
                            backbufferPixels[pixelBaseIdx + 3u] = pxColour.a; // A
                        }
                    });
            }

            renderJobPool->WaitUntilFinished();

#else
            for (size_t i = 0; i < gNumPixels; ++i)
            {
                uvx = float(i % gWindowWidth) / float(gWindowWidth);
                uvy = float(i / gWindowWidth) / float(gWindowHeight);

                pxColour = raytracer.RenderPixel(scene_spheres, { uvx, uvy }, gAspectRatio);

                const size_t pixelBaseIdx = i * 4u;

                backbufferPixels[pixelBaseIdx] = pxColour.b; // B
                backbufferPixels[pixelBaseIdx + 1u] = pxColour.g; // G
                backbufferPixels[pixelBaseIdx + 2u] = pxColour.r; // R
                backbufferPixels[pixelBaseIdx + 3u] = pxColour.a; // A
            }
#endif
        }

        msRender += Influx::Time::GetMillisecondsBetween<double>(Influx::Time::Now(), beforeRender) * (1.0 / gNumFramesPerLog);

        beforePresent = Influx::Time::Now();
        SDL_BlitSurface(backbuffer_surface, NULL, window_surface, NULL);
        SDL_UpdateWindowSurface(window);
        msPresent += Influx::Time::GetMillisecondsBetween<double>(Influx::Time::Now(), beforePresent) * (1.0 / gNumFramesPerLog);

        if (currentFrame > 0 && currentFrame % gNumFramesPerLog == 0)
        {
            printf("\x1b[1F");
            printf("\x1b[1F");
            printf("\x1b[1F");
            printf("\x1b[1F");
            printf("\x1b[1F");

            const double totalAverageTime = msPresent + msRender;
            const double fps = (1 / totalAverageTime * 1000);
            const uint64_t numFPSSamples = (currentFrame / gNumFramesPerLog) + 1;
            sumFPS += fps;

            std::cout << "-- FPS: " << fps << " \n";
            std::cout << "Ms Total: " << totalAverageTime << "\n";
            std::cout << "Ms Render: " << msRender << "\n";
            std::cout << "Ms Present: " << msPresent << "\n";
            std::cout << "-- Avg FPS: " << sumFPS / numFPSSamples << " \n";

            // Reset average timers...
            msPresent = 0.0f;
            msRender = 0.0f;
        }

        ++currentFrame;
    }

#if THREADED_RENDERING
    delete renderJobPool;
    renderJobPool = nullptr;
#endif

    SDL_FreeSurface(backbuffer_surface);
}