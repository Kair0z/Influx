#pragma once

#include "influx_application.h"

#include "core/platform/platform.h"
#include "core/platform/window.h"
#include "core/singleton.h"
#include "core/threading/thread.h"

#include "influx_platform/window.h"
#include "influx_platform/application.h"

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

		// user functions
		void set_on_initialize(const init_callback& clb);
		void set_on_imgui(const imgui_callback& clb);
		void set_on_shutdown(const shutdown_callback& clb);

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

		thread m_input_thread;

		init_callback m_user_init_clb;
		imgui_callback m_user_imgui_clb;
		shutdown_callback m_user_shutdown_clb;

		void process_run_args(const run_args& args);
		void initialize(const run_args& args);
		void cleanup();
		void on_imgui();
	};
}


