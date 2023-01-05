
#if _DEBUG
#pragma comment (lib, "SDL2d.lib")
#else
#pragma comment (lib, "SDL2.lib")
#endif

// SDL Rendering
#include "SDL/SDL.h"

// Influx Engine
#include "Engine.h"
#include "Core/BasicTypes.h"
#include "Core/Math/Vector.h"
#include "Core/String.h"
#include "Core/Container/Array.h"

using namespace Influx;

#pragma region SDL
#ifdef main
#undef main
#endif

struct SDLObject final
{
	SDL_Window* mp_sdlWindow;
	SDL_Surface* mp_sdlWindowSurface;
	SDL_Surface* mp_sdlBackbufferSurface;

	float* mp_depthBufferPixels;
	uint8* mp_backbufferPixels;
};
SDLObject SetupSDL(const Math::Vectoru2& windowSize, const String& title)
{
    SDLObject result{};

    FLX_ASSERT(SDL_Init(SDL_INIT_VIDEO) >= 0);

    result.mp_sdlWindow = SDL_CreateWindow(title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        windowSize.x, windowSize.x, 0);
    FLX_ASSERT(result.mp_sdlWindow);

    result.mp_sdlWindowSurface = SDL_GetWindowSurface(result.mp_sdlWindow);
	FLX_ASSERT(result.mp_sdlWindowSurface);

    // Create a backbuffer to blit into our window...
    result.mp_sdlBackbufferSurface = SDL_DuplicateSurface(result.mp_sdlWindowSurface);
	result.mp_depthBufferPixels = new float[windowSize.x * windowSize.y] {};
	result.mp_backbufferPixels = static_cast<uint8*>(result.mp_sdlBackbufferSurface->pixels);

    return result;
}
#pragma endregion

namespace GlobalSettings
{
	// Window
	constexpr size_t  gWindowWidth = 640;
	constexpr size_t  gWindowHeight = 480;

	constexpr size_t  gNumPixels = gWindowWidth * gWindowHeight;
	constexpr float     gAspectRatio = static_cast<float>(gWindowWidth) / static_cast<float>(gWindowHeight);

	// Scene

	// Threading

	// Logging
	constexpr uint32_t  gNumFramesPerLog = 10;

	// Profiler
	constexpr uint32_t  gNumFramesPerAverage = 60;
}


int main()
{
	using namespace Influx;

	SDLObject sdl = SetupSDL(
		{ GlobalSettings::gWindowWidth, GlobalSettings::gWindowHeight }, "Raytracer");

	Engine engine = Engine();

	
}