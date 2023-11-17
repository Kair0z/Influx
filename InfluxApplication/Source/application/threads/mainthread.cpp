#include "app_pch.h"
#include "mainthread.h"
#include "application/application.h"

#include "influx_renderer.h"

#pragma region imgui
#include "foreign/ImGui/imgui.h"
#if INFLUX_APP_USES_WINDOWS
	#include "foreign/ImGui/imgui_impl_win32.h"
	#include "core/platform/windows_platform.h"
#endif
#pragma endregion

namespace influx::application
{
	void mainthread::static_initialize()
	{
		uint64 editor_frame = 0u;

		while (!renderer::is_initialized_imgui())
		{
			// wait a bit :)
			// if the renderer never initializes imgui,
			// this thread will be useless anyhow...
		}

		ImGui_ImplWin32_Init(m_windowhandle);
	}

	void mainthread::static_tick()
	{
		// window events
		{
			vector<platform::e_windowevent> out_events{};
			if (!platform::poll_window_events(out_events, m_windowhandle))
			{
				application::get_instance().request_quit();
			}

			// handle events
			for (platform::e_windowevent e : out_events)
			{
			}
		}

		// editor:
		{
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			{
				ImGui::ShowDemoWindow();
			}
			ImGui::Render(); // endframe + submits draw data

			ImGui::GetDrawData();
		}
	}

	void mainthread::static_cleanup()
	{
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
}