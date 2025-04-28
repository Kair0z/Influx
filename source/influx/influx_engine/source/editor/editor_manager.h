#pragma once

// influx::core
#include "core/container/map.h"
#include "core/time.h"
#include "core/result.h"

// influx::input
#include "influx_input.h"

// influx::file
#include "influx_file.h"

// influx::imgui
#include "influx_imgui/imgui_widgets.h" // imgui::popup_radial

// influx::engine
#include "editor_window.h"
#include "editor_common.h"
#include "scene_editor/scene_editor.h"

// imgui
struct ImGuiContext;

namespace influx::engine::editor
{
	class editor_manager final
	{
	public:
		editor_manager();
		~editor_manager();

		// editor imgui configuration
		void on_imgui(ImGuiContext& ctx);

		// register an imgui window to run
		template <typename _t>
		static _t& static_window(const string& tag);

		// files
		bool has_project() const;
		string get_projectname() const;
		string get_editor_filepath() const;
		void save_editor();
		void load_editor();
		files::editorfile& get_editorfile();

		float get_mainmenu_height() const;

	private:
		math::vectorf2 m_mousepos;
		bool m_is_mainmenu_active = false;

		scene_editor m_scene_editor;
		static umap<string, editor_window*> m_static_windows;

		files::projectfile m_projectfile;
		files::editorfile m_editorfile;

		void update_inputs();
		void update_context();
		void update_mainmenu();
		void update_background_dockspace();
		void update_static_windows();
	};

	template<typename _t>
	inline _t& editor_manager::static_window(const string& title)
	{
		static bool first = true;
		static _t static_win = _t{};
		if (first)
		{
			m_static_windows[title] = &static_win;
			first = false;
		}

		return static_win;
	}
}