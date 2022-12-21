#pragma once
#include "Common.h"

namespace Influx::Application
{
	class Application final
	{
	public:
		struct Settings final
		{
			String Name = "InfluxApp";

			bool HasWindow = false;
			Math::Vectoru2 WindowDimensions = { 640u, 480u };

			bool HasUI = false;

			bool HasUpdate = false;
		};

	private:
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

		Platform::WindowHandle m_windowHandle;
		Platform::InstanceHandle m_appInstanceHandle;
		Platform::ProcessHandle m_processHandle;

	public:
		Application(const Settings& creationSettings);
		
		void Run(int argc, char** argv);
		void SetQuit();

		bool GetHasStarted() const;
		bool GetShouldQuit() const;
		bool GetHasWindow() const;
		bool GetHasUIRenderer() const;
		bool GetHasUpdate() const;
		bool GetHasCleanedUp() const;
		bool GetHasCreatedWindow() const;

		const Settings& GetSettings() const;
		const Settings& GetCreationSettings() const;

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
		void UIRender();

		void CreateWindow();
	};
}


