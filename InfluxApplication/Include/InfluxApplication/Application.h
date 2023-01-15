#pragma once

#include "InfluxApplication/Common.h"
#include "InfluxRenderer/Renderer.h"

namespace Influx
{
	class Engine;
}

namespace Influx::Application
{
	class Application final
	{
		using EnginePtr			= Ptr<Influx::Engine>;

	public:
		struct Settings final
		{
			String Name = "InfluxApp";

			bool HasWindow = false;
			Math::Vectoru2 WindowDimensions = { 640u, 480u };

			bool HasUI = false;
			bool HasUpdate = false;
			bool HasSceneRender = false;
		};

		Application(const Settings& creationSettings);

		/* Runs the Application, this will only return with SetQuit(); */
		void Run(int argc, char** argv);

		/* Requests the Application to quit running. */
		void SetQuit();

		bool GetHasStarted() const;
		bool GetShouldQuit() const;
		bool GetShouldHaveWindow() const;
		bool GetShouldHaveUI() const;
		bool GetShouldRenderScene() const;
		bool GetHasUpdate() const;
		bool GetHasCleanedUp() const;
		bool GetHasCreatedWindow() const;
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
		Renderer::RootRenderer m_appRenderer;

		bool m_isInitialized;
		bool m_hasStarted;
		bool m_shouldQuit;
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
		void SceneRender();
		void UIRender();

		void CreateWindow();
		void CreateEngine();
		void CreateRenderer();
	};
}


