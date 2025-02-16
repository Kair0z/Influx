#pragma once

// influx::core
#include "core/container/map.h"

// influx::engine
#include "window/window_manager.h"

// imgui
struct ImVec2;
struct ImGuiViewport;

namespace influx::engine
{
	class imgui_manager final
	{
	public:
		imgui_manager();

		void on_window_resize(const math::vectoru2& new_size);
		void new_frame();
		void render();
		void present();

		~imgui_manager();

	private:
		void initialize_font_atlas();
		void initialize_multiviewport();
		void initialize_input();
		void update_monitors();

		static void create_window(ImGuiViewport*);
		static void destroy_window(ImGuiViewport*);
		static void show_window(ImGuiViewport*);
		static void set_windowpos(ImGuiViewport*, ImVec2);
		static ImVec2 get_windowpos(ImGuiViewport*);
		static void set_windowsize(ImGuiViewport*, ImVec2);
		static ImVec2 get_windowsize(ImGuiViewport*);
		static void set_windowfocus(ImGuiViewport*);
		static bool get_windowfocus(ImGuiViewport*);
		static bool get_windowminimized(ImGuiViewport*);
		static void set_windowtitle(ImGuiViewport*, const char*);
		static void set_windowalpha(ImGuiViewport*, float);
		static void update_window(ImGuiViewport* viewport);
		static float get_window_dpi(ImGuiViewport* viewport);
		static void on_changed_viewport(ImGuiViewport* viewport);

		struct viewport_data final
		{
			window_manager::window_id m_window_id = window_manager::k_invalid_id;
		};
		umap<uint32, viewport_data> m_viewports{};
	};
}