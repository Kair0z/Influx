#pragma once

#include "../ImGui/imgui.h"

#include "Common.h"

namespace Influx
{
	enum class EApplicationType
	{
		Default
	};

	struct ApplicationDescription
	{
		EApplicationType Type			= EApplicationType::Default;
		String Name						= "InfluxApp";
		Math::Vectoru2 InitWindowSize	= { 640u, 480u };
	};

	class Application final
	{
		typedef void (*OnUpdateCallback)(Application&);
		typedef void (*OnUIRenderCallback)(const Application&);

	public:
		Application(int argc, char** argv, const ApplicationDescription& desc);
		
		void SetUIRenderCallback(OnUIRenderCallback newClb);
		void SetUpdateCallback(OnUpdateCallback newClb);

		void Run();
		void Quit();

		Application(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(const Application&) = delete;
		Application& operator=(Application&&) = delete;
		~Application();

	private:
		bool m_hasStarted;
		bool m_shouldQuit;

		OnUIRenderCallback	m_uiRenderClb;
		OnUpdateCallback	m_updateClb;

		ApplicationDescription m_initDescription;

		bool AreRequiredCallbacksRegistered() const;
		void InitializeRenderer();

		void OnWindowResize();

#if PLATFORM_WINDOWS
		HWND m_windowHandle;
#endif
	};
}


