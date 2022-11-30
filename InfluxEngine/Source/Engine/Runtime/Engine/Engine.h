#pragma once

#include <thread>
#include <atomic>

namespace Influx
{
	class SceneRenderer;

	class Engine final
	{
	public:
		void Run();
		void Quit();

		bool IsQuit() const;

		void AttachRenderer(SceneRenderer* sceneRenderer);

		Engine() = default;
		Engine(const Engine&) = delete;
		Engine(Engine&&) = delete;
		Engine& operator=(const Engine&) = delete;
		Engine& operator=(Engine&&) = delete;
		~Engine();

		bool IsSceneRendererAttached() const;

		enum class EOptionalAttachment
		{
			SceneRenderer,
			Application
		};

	private:
		SceneRenderer* mp_sceneRenderer;

		uint64_t m_frame{};
		std::atomic_int64_t m_atomic_rtFrame{};
		std::atomic_bool mb_atomic_isQuit{ false };

	private:
		// Threads:
		std::thread m_renderThread;

	private:
		void GameThread_Tick();
		void RenderThread_Tick();
		void PresentToSwapchain();
	};
}


