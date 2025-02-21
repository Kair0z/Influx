#include "engine_pch.h"
#include "scene_editor.h"

// influx::engine
#include "input/input_manager.h"

namespace influx::engine::editor
{
	void scene_editor::on_imgui(ImGuiContext& ctx)
	{
	}

	void scene_editor::on_mouse_down(input::e_mouse_button button, const input::mouse_position& position)
	{
		switch (button)
		{
		case input::e_mouse_button::right:
			m_edit_radial.set_visible(true);
			m_edit_radial.set_position(position.m_client);
			break;
		}
	}

	void scene_editor::on_mouse_up(input::e_mouse_button button, const input::mouse_position& position)
	{
		switch (button)
		{
		case input::e_mouse_button::right:
			m_edit_radial.set_visible(false);
			break;
		}
	}
}