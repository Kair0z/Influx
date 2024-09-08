#pragma once

#include "influx_application.h"

#include "core/platform/platform.h"
#include "core/platform/window.h"
#include "core/singleton.h"

namespace influx::application
{
	class editor;
	class scene;
	class content_manager;
	class renderer;

	class application final
		: public singleton<application>
	{
	public:
		void run(const run_args& args);

		void request_quit();

		static string get_resource_directory();
		static string get_assets_directory();
		static string get_intermediate_directory();

		run_args get_run_arguments() const;

		platform::window_handle get_window_handle() const;
		platform::instance_handle get_instance_handle() const;

		static bool is_quit_requested();
		static bool is_vsync();
		static bool is_editor_enabled();
		static bool is_game_enabled();
		static bool is_scene_render_enabled();
		static bool is_commandlet();

		static editor* get_editor();

	private:

		platform::window_handle m_windowhandle = nullptr;
		platform::instance_handle m_instancehandle = nullptr;

		uint64 m_frame = 0u;
		std::atomic_bool m_is_quit_requested = false;

		run_args m_run_args{};

		string m_resource_dir{};
		string m_asset_dir{};
		string m_int_dir{};

		bool m_staged{};

		renderer* mp_renderer;
		content_manager* mp_content_manager;
		scene* mp_scene;
		editor* mp_editor;

		void process_run_args(const run_args& args);
	};
}


