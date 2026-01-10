#include "engine_pch.h"
#include "editor_window.h"

// influx::imgui
#include "influx_imgui/imgui_translation.h"

// imgui
#include "imgui.h"

namespace influx::engine::editor
{
	void editor_window::run(const callback& clb)
	{
		if (m_is_visible)
		{
			on_prerun();

			if (m_position_locked.is_locked())
				ImGui::SetNextWindowPos(imgui::translate(m_position_locked.get()));

			if (m_size_locked.is_locked())
				ImGui::SetNextWindowSize(imgui::translate(m_size_locked.get()));

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
		if (m_position_locked.is_locked())
		{
			return m_position_locked.m_force_value.value();
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
		if (m_size_locked.is_locked())
		{
			return m_size_locked.m_force_value.value();
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
		m_position_locked.lock(new_position);
	}

	void editor_window::set_size(const math::float2& new_size)
	{
		m_size_locked.lock(new_size);
	}

	editor_window::lockable<math::float2>& editor_window::get_size_prop()
	{
		return m_size_locked;
	}

	editor_window::lockable<math::float2>& editor_window::get_position_prop()
	{
		return m_position_locked;
	}
}