#include "app_pch.h"
#include "editor.h"

// influx::input
#include "influx_input.h"

// imgui
#include "imgui/imgui.h"

namespace influx::application
{
	editor::editor()
	{
		// create ImGui context
		ImGui::CreateContext();

		// Build texture atlas
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

		input::subscribe([this, &io](const input::mouse_event& ev)
		{
			switch (ev.m_type)
			{
			case input::mouse_event::e_type::move:
			{
				bool want_absolute_pos = (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0;
				if (want_absolute_pos)
				{
					ImGui::GetIO().AddMousePosEvent(ev.m_position_screen.x, ev.m_position_screen.y);
				}
				else
				{
					ImGui::GetIO().AddMousePosEvent(ev.m_position_client.x, ev.m_position_client.y);
				}
			}
			break;

			case input::mouse_event::e_type::leave:
			{
				io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
			}
			break;

			case input::mouse_event::e_type::scroll:
			{
				io.AddMouseWheelEvent(0.0f, ev.m_wheel_delta);
			}
			break;

			case input::mouse_event::e_type::button_down:
			{
				int button_value = 0;
				switch (ev.m_button)
				{
				case input::mouse_event::e_button::left: button_value = 0; break;
				case input::mouse_event::e_button::middle: button_value = 2; break;
				case input::mouse_event::e_button::right: button_value = 1; break;
				}

				io.AddMouseButtonEvent(button_value, true);
			}
			break;

			case input::mouse_event::e_type::button_up:
			{
				int button_value = 0;
				switch (ev.m_button)
				{
				case input::mouse_event::e_button::left: button_value = 0; break;
				case input::mouse_event::e_button::middle: button_value = 2; break;
				case input::mouse_event::e_button::right: button_value = 1; break;
				}

				io.AddMouseButtonEvent(button_value, false);
			}
			break;
			}
		});
	}

	void editor::set_window_dimensions(const math::vectorf2& dimensions)
	{
		ImGui::GetIO().DisplaySize = { (float)dimensions.x, (float)dimensions.y };
	}

	void editor::update()
	{
	}

	ImDrawData* editor::get_imgui_drawdata()
	{
		

		ImGui::GetIO();

		ImGui::NewFrame();

		// ImGui::ShowDemoWindow();

		// calls EndFrame
		ImGui::Render();
		return ImGui::GetDrawData();
	}
}