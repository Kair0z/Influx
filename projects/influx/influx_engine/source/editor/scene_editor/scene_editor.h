#pragma once

// influx::input
#include "influx_input.h"

// influx::imgui
#include "influx_imgui/widgets/popup_radial.h"

// imgui
struct ImGuiContext;

namespace influx::engine::editor
{
	class scene_editor final
	{
	public:
		scene_editor();
		~scene_editor();

		void on_imgui(ImGuiContext& ctx);

		void on_mouse_down(input::e_mouse_button button, const input::mouse_position& position);
		void on_mouse_up(input::e_mouse_button button, const input::mouse_position& position);

	private:
		typedef void (*on_radial_select)();
		imgui::popup_radial<on_radial_select, 4u> m_edit_radial{};

		static void on_edit_place();
		static void on_edit_remove();

		void pick_scene(const input::mouse_position& position) const;
	};
}