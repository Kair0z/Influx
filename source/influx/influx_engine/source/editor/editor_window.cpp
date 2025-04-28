#include "engine_pch.h"
#include "editor_window.h"

// influx::imgui
#include "influx_imgui/imgui_translation.h"

// imgui
#include "imgui/imgui.h"

namespace influx::engine::editor
{
	void editor_window::run(const callback& clb)
	{
		if (m_is_visible)
		{
			on_prerun();

			if (m_force_position.is_forced())
				ImGui::SetNextWindowPos(imgui::translate(m_force_position.get()));

			if (m_force_size.is_forced())
				ImGui::SetNextWindowSize(imgui::translate(m_force_size.get()));

			const string& name = m_title.empty() ? "-" : m_title;
			if (ImGui::Begin(name.c_str()))
			{
				m_last_position = imgui::translate(ImGui::GetWindowPos());
				m_last_size = imgui::translate(ImGui::GetWindowSize());

				if (clb != nullptr) clb();

				on_run();
			}

			ImGui::End();
		}
	}

	math::rectf editor_window::get_rect() const
	{
		return math::rectf(get_position(), get_size());
	}

	void editor_window::set_name(const string& name)
	{
		m_title = name;
	}

	string editor_window::get_name() const
	{
		return m_title.empty() ? "-" : m_title;
	}

	math::float2 editor_window::get_position() const
	{
		math::float2 position = {};
		if (m_force_position.is_forced())
		{
			return m_force_position.m_force_value.value();
		}
		else
		{
			return m_last_position;
		}
		
		return position;
	}

	math::float2 editor_window::get_size() const
	{
		math::float2 size = {};
		if (m_force_size.is_forced())
		{
			return m_force_size.m_force_value.value();
		}
		else
		{
			return m_last_size;
		}

		return size;
	}

	void editor_window::toggle()
	{
		set_visible(!is_visible());
	}

	void editor_window::set_visible(bool new_visible)
	{
		m_is_visible = new_visible;
	}

	bool editor_window::is_visible() const
	{
		return m_is_visible;
	}

	void editor_window::set_position(const math::float2& new_position)
	{
		m_force_position.force(new_position);
	}

	void editor_window::set_size(const math::float2& new_size)
	{
		m_force_size.force(new_size);
	}

	editor_window::optional_property<math::float2>& editor_window::get_size_prop()
	{
		return m_force_size;
	}

	editor_window::optional_property<math::float2>& editor_window::get_position_prop()
	{
		return m_force_position;
	}
}