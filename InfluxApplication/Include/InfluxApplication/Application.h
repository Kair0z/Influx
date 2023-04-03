#pragma once

#pragma once

#ifndef __INFLUX_APPLICATION_H_
#define __INFLUX_APPLICATION_H_

#ifdef _DEBUG
#define INFLUX_APPLICATION_DEBUG 1
#else
#define INFLUX_APPLICATION_DEBUG 0
#endif

#define INFLUX_APPLICATION_PLATFORM_WINDOWS 1
#include "Core/Platform/WindowsPlatform.h"

#define INFLUX_APPLICATION_USE_CORE 1

#define INFLUX_APPLICATION_RENDER_D3D12		INFLUX_APPLICATION_PLATFORM_WINDOWS
#define INFLUX_APPLICATION_RENDER_VULKAN	!INFLUX_APPLICATION_RENDER_D3D12

#define FLX_APP_KEEP_TIMING_STATS	0

#define INFLUX_APPLICATION_USE_CORE 1

#if INFLUX_APPLICATION_USE_CORE
#include "Core/BasicTypes.h"
#include "Core/String.h"
#include "Core/Math/Vector.h"
#include "Core/Container/List.h"
#include "Core/Container/Vector.h"
#include "Core/Container/Array.h"
#include "Core/Time.h"
#include "Core/Pointer.h"
#include "Core/Scene/Scene.h"
#else
static_assert(false, "Error: Application requires using the Influx Core-header-library! ")
#endif

#if INFLUX_APPLICATION_DEBUG
// INFLUX_APPLICATION_TODO
#define INFLUX_APPLICATION_TODO __debugbreak();

// INFLUX_APPLICATION_ASSERT
#if INFLUX_APPLICATION_USE_CORE
#include "Core/Assert.h"
#define INFLUX_APPLICATION_ASSERT(x) FLX_ASSERT(x); 
#else
#include <cassert>
#define INFLUX_APPLICATION_ASSERT(x) assert(x);
#endif

#endif

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
		void Run(int argc = 0, char** argv = nullptr);

		/* Requests the Application to quit running. */
		void SignalQuit();

		// Run();
		bool IsRunning() const;

		// Run() > Start();
		bool GetHasStarted() const;

		// SignalQuit();
		bool GetHasRecievedQuit() const;

		// Settings::HasWindow
		bool GetShouldHaveWindow() const;

		// Settings::HasImGUI
		bool GetShouldHaveImgui() const;

		// Settings::HasWindow && Settings::HasSceneRender
		bool GetShouldRenderScene() const;

		// Run() > Cleanup();
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

#endif