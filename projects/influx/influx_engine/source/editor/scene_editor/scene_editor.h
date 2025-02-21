#pragma once

// influx::input
#include "influx_input.h"

// influx::imgui
#include "influx_imgui/imgui_widgets.h" // imgui::popup_radial

// imgui
struct ImGuiContext;

namespace influx::engine::editor
{
	class scene_editor final
	{
	public:
		void on_imgui(ImGuiContext& ctx);

		void on_mouse_down(input::e_mouse_button button, const input::mouse_position& position);
		void on_mouse_up(input::e_mouse_button button, const input::mouse_position& position);

	private:
		imgui::popup_radial<function<void()>> m_edit_radial{};
	};
}