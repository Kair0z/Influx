#include "engine_pch.h"
#include "scene_editor.h"

// influx::engine
#include "input/input_manager.h"
#include "world/world.h"
#include "scene/scene.h"
#include "component/component.h"

namespace influx::engine::editor
{
	void scene_editor::on_edit_place()
	{
		scene& scene = get_engine()->get_current_scene();
		scene.create_entity();
	}

	void scene_editor::on_edit_remove()
	{
		
	}

	scene_editor::scene_editor()
	{
		m_edit_radial.set_radius(60.0f);
		m_edit_radial.set_item("place", scene_editor::on_edit_place);
		m_edit_radial.set_item("remove", scene_editor::on_edit_remove);
	}

	scene_editor::~scene_editor()
	{
	}

	void scene_editor::on_imgui(ImGuiContext& ctx)
	{
		input_manager& inputman = get_engine()->get_input();
		m_edit_radial.render(inputman.get_mouse_position_client());

		if (m_edit_radial.has_selection())
		{
			on_radial_select* ptr_ptr = m_edit_radial.get_selected();
			if (ptr_ptr) (*ptr_ptr)();
		}
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