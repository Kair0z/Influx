#pragma once

// influx::core
#include "core/container/vector.h"
#include "core/string.h"

// influx::imgui
#include "influx_imgui/imgui_translation.h"
#include "influx_imgui/widgets/popup_radial.h"

namespace influx::imgui
{
	struct scoped_style_var final
	{
	public:
		explicit scoped_style_var(ImGuiStyleVar style, const float& value)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, value);
		}

		explicit scoped_style_var(ImGuiStyleVar style, const ImVec2& value)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, value);
		}

		~scoped_style_var()
		{
			ImGui::PopStyleVar();
		}
	};
}