#include "pch.h"
#include "Engine.h"

#include "Engine/Runtime/Logger/Logger.h"

// Windows:
#if PLATFORM_WINDOWS
#include "Core/Platform/WindowsPlatform.h"
#endif

#define LOG_ENGINE_INIT 1
#define LOG_ENGINE_FRAMES 0

namespace Influx
{
	void Engine::Run()
	{
		Logger::Info("Engine:Init");
		{

		}

#if FLX_THREADED_RENDERING
		m_renderThread = std::thread([&] 
			{ 
				RenderThread_Tick(); 
			});
#endif

		// [Engine-Loop]
		Logger::Info("Engine:Start");
		while (!mb_atomic_isQuit)
		{
#if LOG_ENGINE_FRAMES
			Logger::Info("Tick:Engine[{}]", m_frame);
#endif

			GameThread_Tick();

#if !FLX_THREADED_RENDERING
			RenderThread_Tick();
#endif
			PresentToSwapchain();

			++m_frame;
		}

		Logger::Info("Engine:Destroy");
		{

		}
	}

	void Engine::GameThread_Tick()
	{
#if LOG_ENGINE_FRAMES
		Logger::Info("Tick:Game[{}]", m_frame);
#endif
	}

	void Engine::RenderThread_Tick()
	{
#if LOG_ENGINE_FRAMES
		Logger::Info("Tick:Render[{}]", m_frame);
#endif

		Logger::Info("RT{}, ms: {}", m_frame, 0.0f);

		++m_atomic_rtFrame;
	}

	void Engine::PresentToSwapchain()
	{
#if LOG_ENGINE_FRAMES
		Logger::Info("Tick:Present[{}]", m_frame);
#endif
	}

	Engine::~Engine()
	{

	}

	bool Engine::IsSceneRendererAttached() const
	{
		return false;
	}

	void Engine::Quit()
	{
		mb_atomic_isQuit = true;
	}

	bool Engine::IsQuit() const
	{
		return mb_atomic_isQuit;
	}

	void Engine::AttachRenderer(SceneRenderer* sceneRenderer)
	{
		if (mp_sceneRenderer != nullptr)
		{
			// Detach previous scene renderer...?
		}

		mp_sceneRenderer = sceneRenderer;
	}
}

