#include "app_pch.h"
#include "layer_editor.h"

#include "application/application_backend.h"
#if INFLUX_PLATFORM_WINDOWS
#include "imgui/imgui_impl_win32.h"
#endif

#include "ImGui/imgui.h"

namespace influx::application
{
	inline static void imgui_frame()
	{
		// main menu
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("file"))
			{
				if (ImGui::MenuItem("save"))
				{

				}

				if (ImGui::MenuItem("open"))
				{

				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("options"))
			{
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		// game viewport
		if (ImGui::Begin("viewport"))
		{

			ImGui::End();
		}

		// demo window
		ImGui::ShowDemoWindow();
	}

	void layer_editor::on_enable()
	{
		ImGui_ImplWin32_Init(application::get_instance().get_window_handle());
	}

	void layer_editor::on_event(layer_event* e)
	{

	}

	void layer_editor::on_tick()
	{
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		imgui_frame();
		ImGui::Render();
	}

	void layer_editor::on_disable()
	{
		ImGui_ImplWin32_Shutdown();
	}
}
