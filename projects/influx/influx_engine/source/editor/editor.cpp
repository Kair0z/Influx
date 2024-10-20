#include "engine_pch.h"
#include "imgui/imgui.h"

namespace influx::engine
{
	void editor_module::on_config(app_config&, editor_config&)
	{
	}

	void editor_module::on_imgui()
	{
		if (ImGui::BeginMainMenuBar())
		{

			ImGui::EndMainMenuBar();
		}
	}

	void editor_module::on_cleanup()
	{
	}
}