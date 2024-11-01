#include "engine_pch.h"
#include "imgui/imgui.h"

namespace influx::engine
{
	void editor_module::on_config(app_config&, editor_config&)
	{
	}

	void editor_module::on_imgui(ImGuiContext& ctx)
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("file"))
			{
				if (ImGui::Button("new"))
				{

				}

				if (ImGui::Button("open"))
				{

				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("edit"))
			{
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void editor_module::on_cleanup()
	{
	}
}