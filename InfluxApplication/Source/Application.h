#pragma once

#include "Renderer.h"
#include "Common.h"

#include "Engine/Runtime/Engine/Engine.h"

namespace Influx::Application
{
	enum class EApplicationType : uint8_t
	{
		Minimal,		// Only has an Update loop
		Windowed,		// Only has an Update loop + Window & events
		ImGuiApp,		// Only has an Update loop + Window & events + Imgui hooks
		Max
	};

	constexpr char const* k_eApplicationTypeStrings[static_cast<uint8_t>(EApplicationType::Max)] =
	{
		"Minimal",
		"Windowed",
		"ImGuiApp"
	};

	class Application final
	{
	public:
		struct Settings final
		{
			EApplicationType Type = EApplicationType::Minimal;
			String Name = "InfluxApp";
			Math::Vectoru2 WindowDimensions = { 640u, 480u };
		};

	private:
		bool m_isInitialized;
		bool m_hasStarted;
		bool m_shouldQuit;

		uint64_t m_frame;
		float m_time;
		float m_deltaTime;

		const Settings& m_creationSettings;
		Settings m_currentSettings;

		class ImGuiRendererDx12* mp_renderer{};
		Engine m_engine{};

	public:
		Application(const Settings& creationSettings);
		
		void Run(int argc, char** argv);
		void Quit();

		virtual void OnUpdate() {};
		virtual void OnUIRender();
		virtual void OnResize() {};

		Application(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(const Application&) = delete;
		Application& operator=(Application&&) = delete;
		virtual ~Application();

		bool GetHasStarted() const;
		bool GetShouldQuit() const;
		bool GetHasWindow() const;
		bool GetHasUIRenderer() const;

		const Settings& GetSettings() const;
		const Settings& GetCreationSettings() const;

	private:
		void Initialize();
		void Cleanup();
		void PollWindowEvents();
		void Update();

		void UIRender();
		void UIRender_ApplicationUI();
		void UIRender_AppUI_FileMenu();
		void UIRender_AppUI_AppInfo();
		void UIRender_AppUI_EngineLog();

		void UIElement(const Settings& settings);

		void CreateWindow();
		void ShutdownOtherApplication();

		static Application* sp_currentApplicationInstance;

#if PLATFORM_WINDOWS
		::HWND m_windowHandle;
		static LRESULT CALLBACK WindowsProcedure(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam);
#elif PLATFORM_TESTNULL
		// 
#endif
	};
}


