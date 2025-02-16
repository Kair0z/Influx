#include "engine_pch.h"
#include "imgui_manager.h"

// influx::input
#include "influx_input.h"

// imgui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"

namespace influx::engine
{
	inline ImGuiKey translate(const input::e_key key)
	{
		switch (key)
		{
		case input::e_key::left:		return ImGuiKey::ImGuiKey_LeftArrow;
		case input::e_key::right:		return ImGuiKey::ImGuiKey_RightArrow;
		case input::e_key::down:		return ImGuiKey::ImGuiKey_DownArrow;
		case input::e_key::up:			return ImGuiKey::ImGuiKey_UpArrow;
		case input::e_key::lshift:		return ImGuiKey::ImGuiKey_LeftShift;
		case input::e_key::rshift:		return ImGuiKey::ImGuiKey_RightShift;
		case input::e_key::lctrl:		return ImGuiKey::ImGuiKey_LeftCtrl;
		case input::e_key::rctrl:		return ImGuiKey::ImGuiKey_RightCtrl;
		case input::e_key::space:		return ImGuiKey::ImGuiKey_Space;
		case input::e_key::backspace:	return ImGuiKey::ImGuiKey_Backspace;
		case input::e_key::enter:		return ImGuiKey::ImGuiKey_Enter;
		case input::e_key::home:		return ImGuiKey::ImGuiKey_Home;
		case input::e_key::end:			return ImGuiKey::ImGuiKey_End;
		case input::e_key::insert:		return ImGuiKey::ImGuiKey_Insert;
		case input::e_key::deleet:		return ImGuiKey::ImGuiKey_Delete;
		case input::e_key::apostrophe:	return ImGuiKey::ImGuiKey_Apostrophe;
		case input::e_key::comma:		return ImGuiKey::ImGuiKey_Comma;
		case input::e_key::minus:		return ImGuiKey::ImGuiKey_Minus;
		case input::e_key::period:		return ImGuiKey::ImGuiKey_Period;
		case input::e_key::backslash:	return ImGuiKey::ImGuiKey_Backslash;
		case input::e_key::slash:		return ImGuiKey::ImGuiKey_Slash;
		case input::e_key::semicolon:	return ImGuiKey::ImGuiKey_Semicolon;
		case input::e_key::equal:		return ImGuiKey::ImGuiKey_Equal;
		case input::e_key::lbracket:	return ImGuiKey::ImGuiKey_LeftBracket;
		case input::e_key::rbracket:	return ImGuiKey::ImGuiKey_RightBracket;
			// case e_key::plus:		return ImGuiKey::;
		}

		return ImGuiKey::ImGuiKey_None;
	}

	imgui_manager::imgui_manager()
	{
		// create ImGui context
		ImGui::CreateContext();

		// Build texture atlas
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

		// docking
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
		
#if INFLUX_PLATFORM_WINDOWS
		platform::window& main_window = get_engine()->get_windowman().get_main_window();
		ImGui_ImplWin32_Init(main_window.get_platform_handle());

		// mouse events
		input::subscribe([this, &io](const input::mouse_event& ev)
		{
			switch (ev.m_type)
			{
			case input::mouse_event::type::move:
			{
				bool want_absolute_pos = (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0;
				if (want_absolute_pos)
				{
					io.AddMousePosEvent(ev.m_position.m_screen.x, ev.m_position.m_screen.y);
				}
				else
				{
					io.AddMousePosEvent(ev.m_position.m_client.x, ev.m_position.m_client.y);
				}
			}
			break;

			case input::mouse_event::type::leave:
			{
				io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
			}
			break;

			case input::mouse_event::type::scroll:
			{
				io.AddMouseWheelEvent(0.0f, ev.m_wheel_delta);
			}
			break;

			case input::mouse_event::type::button_down:
			{
				int button_value = 0;
				switch (ev.m_button)
				{
				case input::e_mouse_button::left: button_value = 0; break;
				case input::e_mouse_button::middle: button_value = 2; break;
				case input::e_mouse_button::right: button_value = 1; break;
				}

				io.AddMouseButtonEvent(button_value, true);
			}
			break;

			case input::mouse_event::type::button_up:
			{
				int button_value = 0;
				switch (ev.m_button)
				{
				case input::e_mouse_button::left: button_value = 0; break;
				case input::e_mouse_button::middle: button_value = 2; break;
				case input::e_mouse_button::right: button_value = 1; break;
				}

				io.AddMouseButtonEvent(button_value, false);
			}
			break;
			}
		});

		// keyboard events
		input::subscribe([this, &io](const input::key_event& key)
		{
			const bool is_ascii = key.is_ascii();
			const bool is_key_down = key.m_type != input::key_event::e_type::keyup;
			if (!is_ascii)
			{
				io.AddKeyEvent(translate(key.m_key), is_key_down);
			}
			else
			{
				io.AddInputCharacter(key.m_ascii_char);
			}
		});
#endif
	}

	void imgui_manager::on_window_resize(const math::vectoru2& new_size)
	{
		// update imgui IO
		ImGui::GetIO().DisplaySize = { (float)new_size.x, (float)new_size.y };
	}

	void imgui_manager::new_frame()
	{
#if INFLUX_PLATFORM_WINDOWS
		ImGui_ImplWin32_NewFrame();
#endif
	}

	void imgui_manager::render()
	{
		auto platio = ImGui::GetPlatformIO();
		for (const auto& viewport : platio.Viewports)
		{
			// for each viewport render to a target
			// renderer::draw_imgui(ImGui::GetDrawData(), *mp_scene_target);
		}
	}

	void imgui_manager::present()
	{
		auto platio = ImGui::GetPlatformIO();
#if INFLUX_PLATFORM_WINDOWS
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			// TODO for OpenGL: restore current GL context.

			for (const auto& viewport : platio.Viewports)
			{
				// for each viewport render to a target
				// renderer::draw_imgui(ImGui::GetDrawData(), *mp_scene_target);
			}
		}
#endif
	}

	imgui_manager::~imgui_manager()
	{
#if INFLUX_PLATFORM_WINDOWS
		ImGui_ImplWin32_Shutdown();
#endif
	}
}