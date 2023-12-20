#pragma once

#include "ImGui/imgui.h"
#include "Core/Math/Transform.h"

namespace influx::imgui
{
	struct slider_settings final
	{
		float m_speed = 1.0f;
		float m_min = 0.0f;
		float m_max = 1.0f;
		const char* mp_format = "%.3f";
		ImGuiSliderFlags m_flags{};
	};

	void widget_transform_editor(math::transform3D* transform, const slider_settings& settings = {})
	{
		if (transform != nullptr)
		{
			ImGui::DragFloat3("position",
				transform->get_position().data(),
				settings.m_speed,
				settings.m_min,
				settings.m_max,
				settings.mp_format,
				settings.m_flags);

			math::float3 eulerAngles{};
			ImGui::DragFloat3("rotation",
				eulerAngles.data(),
				settings.m_speed,
				-359.99f, 360.0f,
				settings.mp_format,
				settings.m_flags);

			ImGui::DragFloat3("scale",
				transform->get_scale().data(),
				settings.m_speed,
				0.0f,
				settings.m_max,
				settings.mp_format,
				settings.m_flags);
		}
	}

	bool window_transform_editor(const char* label, math::transform3D* transform, const slider_settings& settings = {})
	{
		if (ImGui::Begin(label))
		{
			widget_transform_editor(transform, settings);
			ImGui::End();

			return true;
		}

		return false;
	}
}
