#pragma once

#include "InfluxApplication/Common.h"

namespace Influx
{
	class Engine;
}

namespace Influx::Renderer
{
	class RootRenderer;
}

namespace Influx::Application
{
	class Application final
	{
		using EnginePtr			= Ptr<Influx::Engine>;
		using RendererPtr		= Ptr<Influx::Renderer::RootRenderer>;

	public:
		struct Settings final
		{
			String Name = "InfluxApp";

			bool HasWindow = false;
			Math::Vectoru2 WindowDimensions = { 640u, 480u };

			bool HasImGUI = false;
			bool HasUpdate = false;
			bool HasSceneRender = false;
		};

		Application(const Settings& creationSettings);

		/* Runs the Application, this will only return with SetQuit(); */
		void Run(int argc, char** argv);

		/* Requests the Application to quit running. */
		void SignalQuit();

		bool GetHasStarted() const;
		bool GetHasRecievedQuit() const;
		bool GetShouldHaveWindow() const;
		bool GetShouldHaveImgui() const;
		bool GetShouldRenderScene() const;
		bool GetShouldHaveUpdate() const;
		bool GetHasCleanedUp() const;
		bool GetHasCreatedWindow() const;
		bool GetHasCreatedRenderer() const;
		bool GetHasCreatedEngine() const;

		const Settings& GetSettings() const;
		const Settings& GetCreationSettings() const;

	private:
		/* Platform Data */
		Platform::WindowHandle m_windowHandle;
		Platform::InstanceHandle m_appInstanceHandle;
		Platform::ProcessHandle m_processHandle;

		/* InfluxEngine */
		EnginePtr mp_engine;

		/* Main App Renderer */
		RendererPtr mp_appRenderer;

		Scene::Scene m_scene;

		bool m_isInitialized;
		bool m_hasStarted;
		bool m_recievedQuit;
		bool m_hasCreatedWindow;
		bool m_hasCleanedUp;

		uint64 m_frame;
		float m_time;
		float m_deltaTime;

		const Settings& m_creationSettings;
		Settings m_currentSettings;

	public:
		Application(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(const Application&) = delete;
		Application& operator=(Application&&) = delete;
		virtual ~Application();

	private:
		void Initialize();
		void Start();
		void Cleanup();
		void PollWindowEvents();
		void Update();
		void Render();
		void ImguiRender();

		void CreateWindow();
		void CreateEngine();
		void CreateRenderer();
	};
}


