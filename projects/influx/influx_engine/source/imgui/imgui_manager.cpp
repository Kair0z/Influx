#include "engine_pch.h"
#include "imgui_manager.h"

// influx::core
#include "core/string.h"

// influx::engine
#include "window/window_manager.h"

// influx::input
#include "influx_input.h"

// influx::platform
#include "influx_platform/window.h"
#include "influx_platform/platform.h"

// influx::renderer
#include "influx_renderer.h"

// imgui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"

namespace influx::engine
{
#define INFLUX_IMGUI_VIEWPORTS 0

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

		// Setup backend capabilities flags
		ImGuiIO& io = ImGui::GetIO();
		io.BackendPlatformUserData = (void*)this;
		io.BackendPlatformName = "imgui_impl_influx";
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		const bool enable_viewports = false;
		if (enable_viewports)
		{
			io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;    // We can create multi-viewports on the Platform side (optional)
			io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport; // We can call io.AddMouseViewportEvent() with correct data (optional)
			io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		}
		
		update_monitors();
		initialize_font_atlas();
		initialize_input();
		initialize_multiviewport();
	}

	void imgui_manager::on_window_resize(const math::vectoru2& new_size)
	{
		// update imgui IO
		ImGui::GetIO().DisplaySize = { (float)new_size.x, (float)new_size.y };
	}

	void imgui_manager::render(const renderer::scene_imgui& scene)
	{
		if (scene.is_empty() == false)
		{
			ImGuiIO& io = ImGui::GetIO();
			window_manager& windowman = get_engine()->get_windowman();
			ImGuiContext* context = ImGui::GetCurrentContext();
			
			// update mouse position and such
			update_mousedata();

			ImGui::NewFrame();
			scene.m_imgui_stacks[0u](*ImGui::GetCurrentContext());
			ImGui::Render();
			ImGui::UpdatePlatformWindows();
			
			// execute the draw for each viewport
			ImGuiPlatformIO& platio = ImGui::GetPlatformIO();
			vector<ImDrawData const*> draws{};  draws.reserve(platio.Viewports.Size);
			vector<renderer::target const*> targets{}; targets.reserve(platio.Viewports.Size);
			for (const auto& viewport : platio.Viewports)
			{
				if (viewport->Flags & ImGuiViewportFlags_IsMinimized)
					continue;

				// get the rendertarget
				viewport_data& data = m_viewports[viewport->ID];
				platform::window& window = windowman.get_window(data.m_window_id);
				auto* window_target = renderer::get_window_target(window);

				// clear if non-main
				const bool is_non_main = !windowman.is_main(data.m_window_id);
				if (is_non_main)
				{
					renderer::clear_target(*window_target, {});
				}

				draws.push_back(viewport->DrawData);
				targets.push_back(window_target);
			}

			renderer::draw_imgui(draws, targets);
		}
	}

	imgui_manager::viewport_data& imgui_manager::get_viewportdata(uint32 id)
	{
		return m_viewports[id];
	}

	void imgui_manager::initialize_font_atlas()
	{
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	}

	static window_manager* g_windowman = nullptr;
	static imgui_manager* g_imguiman = nullptr;

#pragma region multiviewport hooks
	const bool g_mute = true;
	inline static ImVec2 translate(const math::vectoru2& f2)
	{
		return ImVec2((float)f2.x, (float)f2.y);
	}
	inline static ImVec2 translate(const math::vectorf2& f2)
	{
		return ImVec2((float)f2.x, (float)f2.y);
	}
	inline static platform::window_style get_windowstyle(ImGuiViewportFlags flags)
	{
		platform::window_style new_style{};
		new_style.set_decoration((flags & ImGuiViewportFlags_NoDecoration) == false);
		new_style.set_taskicon_enabled((flags & ImGuiViewportFlags_NoTaskBarIcon) == false);
		new_style.set_topmost(flags & ImGuiViewportFlags_TopMost);
		return new_style;
	}
	inline static imgui_manager::viewport_data& get_data(ImGuiViewport* viewport)
	{
		return g_imguiman->get_viewportdata(viewport->ID);
		//return *reinterpret_cast<imgui_manager::viewport_data*>(viewport->PlatformUserData);
	}
	inline static ImGuiViewport* find_viewport(platform::window_handle handle)
	{
		ImGuiPlatformIO& platformio = ImGui::GetPlatformIO();
		for (ImGuiViewport* viewport : platformio.Viewports)
		{
			if (viewport->PlatformHandle == handle) return viewport;
		}
		return nullptr;
	}
	inline static window_manager& get_windowman();
	void imgui_manager::create_window(ImGuiViewport* viewport)
	{
		viewport_data& data = get_data(viewport);
		math::rectu rect = 
		{ 
			(uint32)viewport->Pos.x, 
			(uint32)viewport->Pos.y, 
			(uint32)(viewport->Pos.x + viewport->Size.x), 
			(uint32)(viewport->Pos.y + viewport->Size.y) 
		};

		// create the window
		platform::window_desc new_desc{};
		new_desc.m_name = "viewport_" + to_string(viewport->ID);
		new_desc.m_style = get_windowstyle(viewport->Flags);
		new_desc.m_dimensions = rect.get_dimensions();
		new_desc.m_position = rect.get_leftbottom();
		data.m_window_id = g_windowman->spawn(new_desc).get();

		// setup window event callback
		platform::window& window = g_windowman->get_window(data.m_window_id);
		viewport->PlatformHandle = window.get_platform_handle();
		window.set_event_callback([](const platform::window_event& ev)
		{
			platform::window_handle handle = ev.m_window->get_platform_handle();
			ImGuiViewport* viewport = find_viewport(handle);
			if (viewport)
			{
				switch (ev.m_type)
				{
				case platform::window_event::type::quit: viewport->PlatformRequestClose = true; break;
				case platform::window_event::type::move: viewport->PlatformRequestMove = true; break;
				case platform::window_event::type::size: viewport->PlatformRequestResize = true; break;
				case platform::window_event::type::mouse_activate: break;
				}
			}
		});
	}
	void imgui_manager::destroy_window(ImGuiViewport* viewport)
	{
		viewport_data& data = get_data(viewport);
		g_windowman->destroy(data.m_window_id);
	}
	void imgui_manager::show_window(ImGuiViewport* viewport)
	{
		viewport_data& data = get_data(viewport);
		platform::window& window = g_windowman->get_window(data.m_window_id);
		window.set_visibility(platform::window::e_visibility::showed);
	}
	void imgui_manager::set_windowpos(ImGuiViewport* viewport, ImVec2 position)
	{
		if (!g_mute) logn("viewport:set_windowpos {},{}", position.x, position.y);

		viewport_data& data = get_data(viewport);
		platform::window& window = g_windowman->get_window(data.m_window_id);
		window.set_position({ position.x, position.y });
	}
	ImVec2 imgui_manager::get_windowpos(ImGuiViewport* viewport)
	{
		viewport_data& data = get_data(viewport);
		platform::window& window = g_windowman->get_window(data.m_window_id);
		const math::vectoru2 windowpos = window.get_position();

		if (!g_mute) logn("viewport:get_windowpos {},{}", windowpos.x, windowpos.y);

		return translate(windowpos);
	}
	void imgui_manager::set_windowsize(ImGuiViewport* viewport, ImVec2 size)
	{
		viewport_data& data = get_data(viewport);
		platform::window& window = g_windowman->get_window(data.m_window_id);
		window.set_dimensions({ size.x, size.y });
	}
	ImVec2 imgui_manager::get_windowsize(ImGuiViewport* viewport)
	{
		viewport_data& data = get_data(viewport);
		platform::window& window = g_windowman->get_window(data.m_window_id);
		return translate(window.get_dimensions(platform::window::e_space::client));
	}
	void imgui_manager::set_windowfocus(ImGuiViewport* viewport)
	{
		viewport_data& data = get_data(viewport);
		platform::window& window = g_windowman->get_window(data.m_window_id);
		window.set_foreground();
		window.set_focus();
	}
	bool imgui_manager::get_windowfocus(ImGuiViewport* viewport)
	{
		viewport_data& data = get_data(viewport);
		platform::window& window = g_windowman->get_window(data.m_window_id);
		return window.is_focus();
	}
	bool imgui_manager::get_windowminimized(ImGuiViewport* viewport)
	{
		viewport_data& data = get_data(viewport);
		platform::window& window = g_windowman->get_window(data.m_window_id);
		return window.is_minimized();
	}
	void imgui_manager::set_windowtitle(ImGuiViewport* viewport, const char* new_title)
	{
		viewport_data& data = get_data(viewport);
		platform::window& window = g_windowman->get_window(data.m_window_id);
		window.set_title(new_title);
	}
	void imgui_manager::set_windowalpha(ImGuiViewport* viewport, float alpha)
	{
		viewport_data& data = get_data(viewport);
		platform::window& window = g_windowman->get_window(data.m_window_id);
		window.set_alpha(alpha);
	}
	void imgui_manager::update_window(ImGuiViewport* viewport)
	{
		viewport_data& parent_data = g_imguiman->get_viewportdata(viewport->ParentViewportId);
		viewport_data& data = get_data(viewport);

		platform::window& window = g_windowman->get_window(data.m_window_id);
		platform::window& parent = g_windowman->get_window(parent_data.m_window_id);

		math::rectu rect =
		{
			(uint32)viewport->Pos.x,
			(uint32)viewport->Pos.y,
			(uint32)(viewport->Pos.x + viewport->Size.x),
			(uint32)(viewport->Pos.y + viewport->Size.y)
		};

		window.set_style(get_windowstyle(viewport->Flags));
		window.set_owner(parent);
		window.adjust_rect(rect);
	}
	float imgui_manager::get_window_dpi(ImGuiViewport* viewport)
	{
		viewport_data& data = get_data(viewport);
		platform::window& window = g_windowman->get_window(data.m_window_id);
		return window.get_dpi();
	}
	void imgui_manager::on_changed_viewport(ImGuiViewport*)
	{
		
	}
#pragma endregion

	void imgui_manager::initialize_multiviewport()
	{
		g_imguiman = this;
		g_windowman = &get_engine()->get_windowman();

		// Register main window handle (which is owned by the main application, not by us)
		// This is mostly for simplicity and consistency, so that our code (e.g. mouse handling etc.) can use same logic for main and secondary viewports.
		ImGuiViewport* main_viewport = ImGui::GetMainViewport();
		m_viewports[main_viewport->ID].m_window_id = g_windowman->get_main_id();
		main_viewport->PlatformUserData = &m_viewports.at(main_viewport->ID);
		main_viewport->PlatformHandle = g_windowman->get_main_window().get_platform_handle();

		// Register platform interface
		ImGuiPlatformIO& platio = ImGui::GetPlatformIO();
		platio.Platform_CreateWindow	= create_window;
		platio.Platform_DestroyWindow	= destroy_window;
		platio.Platform_ShowWindow		= show_window;
		platio.Platform_SetWindowPos	= set_windowpos;
		platio.Platform_GetWindowPos	= get_windowpos;
		platio.Platform_SetWindowSize	= set_windowsize;
		platio.Platform_GetWindowSize	= get_windowsize;
		platio.Platform_SetWindowFocus	= set_windowfocus;
		platio.Platform_GetWindowFocus	= get_windowfocus;
		platio.Platform_GetWindowMinimized	= get_windowminimized;
		platio.Platform_SetWindowTitle		= set_windowtitle;
		platio.Platform_SetWindowAlpha		= set_windowalpha;
		platio.Platform_UpdateWindow		= update_window;
		platio.Platform_GetWindowDpiScale	= get_window_dpi;
		platio.Platform_OnChangedViewport	= on_changed_viewport;
	}

	void imgui_manager::initialize_input()
	{
		ImGuiIO& io = ImGui::GetIO();

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
	}

	void imgui_manager::update_monitors()
	{
		ImGuiPlatformIO& platformio = ImGui::GetPlatformIO();
		platformio.Monitors.clear();
		
		for (const platform::monitor& monitor: platform::monitor::query_monitors())
		{
			ImGuiPlatformMonitor imgui_monitor{};
			imgui_monitor.DpiScale = monitor.m_dpi_scale;
			imgui_monitor.MainPos = translate(monitor.m_mainpos);
			imgui_monitor.MainSize = translate(monitor.m_mainsize);
			imgui_monitor.WorkPos = translate(monitor.m_workpos);
			imgui_monitor.WorkSize = translate(monitor.m_worksize);
			imgui_monitor.PlatformHandle = monitor.m_platform_handle;

			if (monitor.m_is_primary)
			{
				platformio.Monitors.push_front(imgui_monitor);
			}
			else
			{
				platformio.Monitors.push_back(imgui_monitor);
			}
		}
	}

	void imgui_manager::update_mousedata()
	{
		platform::window_handle cursored_window = platform::platform::find_window_from_cursor();
		if (cursored_window != nullptr)
		{
			ImGuiViewport* viewport = find_viewport(cursored_window);
			if (viewport)
			{
				ImGui::GetIO().AddMouseViewportEvent(viewport->ID);
			}
		}
	}

	imgui_manager::~imgui_manager()
	{
	}
}