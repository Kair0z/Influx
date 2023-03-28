#pragma once

#include "InfluxApplication/Common.h"

#pragma region Predeclarations
namespace Influx
{
	class Engine;
}

namespace Influx::Renderer
{
	class RootRenderer;
}
#pragma endregion

namespace Influx::Application
{
	class Application final
	{
		using EnginePtr			= Influx::Engine*;
		using RendererPtr		= Influx::Renderer::RootRenderer*;

	public:
		struct Settings final
		{
			String Name = "InfluxApp";

			bool HasWindow = false;
			Math::Vectoru2 WindowDimensions = { 640u, 480u };

			bool HasImGUI = false;
			bool HasUpdate = false;
			bool HasSceneRender = false;
			bool VSync = false;
		};

		struct Time final
		{
			float DeltaTime;
			float TimeSinceCreation;
			float TimeSinceRun;
		};

		/* Creates the Application, this will initialize base-necessary components */
		explicit Application(const Settings& creationSettings);

		/* Runs the Application, the calling thread will only return when SignalQuit(); gets called */
		void Run(int argc, char** argv);

		/* Requests the Application to quit running. */
		void SignalQuit();

		bool GetHasStarted() const;
		bool IsRunning() const;
		bool GetHasRecievedQuit() const;
		bool GetShouldHaveWindow() const;
		bool GetShouldHaveImgui() const;
		bool GetShouldRenderScene() const;
		bool GetShouldHaveUpdate() const;
		bool GetHasCleanedUp() const;
		bool GetHasCreatedWindow() const;
		bool GetHasCreatedRenderer() const;
		bool GetHasCreatedEngine() const;

		const Time& GetTime() const;
		float GetTimeSinceCreation() const;
		float GetTimeSinceRun() const;

		const Settings& GetSettings() const;
		const Settings& GetCreationSettings() const;

	private:
		/* Platform Application-Data */
		Platform::WindowHandle		m_windowHandle;
		Platform::InstanceHandle	m_appInstanceHandle;
		Platform::ProcessHandle		m_processHandle;

		/* Underlying Engine */
		EnginePtr mp_engine;

		/* Renderer */
		RendererPtr mp_appRenderer;

		/* Current scene */
		Scene::Scene m_scene;

		bool m_isInitialized;
		bool m_hasStarted;
		bool m_isRunning;
		bool m_recievedQuit;
		bool m_hasCreatedWindow;
		bool m_hasCleanedUp;

		Time m_time;
		uint64 m_frame;

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

		void CreateWindow();
		void CreateEngine();
		void CreateRenderer();
	};
}


