#pragma once

// influx::core
#include "core/container/vector.h"
#include "core/string.h"
#include "core/math/transform.h"

// influx::imgui
#include "influx_imgui/imgui_translation.h"
#include "influx_imgui/widgets/popup_radial.h"
#include "influx_imgui/widgets/text_editor.h"

// imgui
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

namespace influx::imgui
{
	inline void transform3D(const string& title, const math::transform3D& transform)
	{
		ImGui::Text(title.c_str());

		static const string kfloat_precision = "%.2f";
		static const string ktriple_float = kfloat_precision + "," + kfloat_precision + "," + kfloat_precision;

		const math::float3 position = transform.get_position();
		ImGui::Text(("position: \t" + ktriple_float).c_str(), position.x, position.y, position.z);

		const math::rotation rotation = transform.get_rotation();
		const math::float3 eulers = rotation.get_euler_angles();
		ImGui::Text(("rotation: \t" + ktriple_float).c_str(), eulers.x, eulers.y, eulers.z);

		const math::float3 scale = transform.get_scale();
		ImGui::Text(("scale: \t" + ktriple_float).c_str(), scale.x, scale.y, scale.z);
	}

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