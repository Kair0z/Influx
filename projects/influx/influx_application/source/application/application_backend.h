#pragma once

#include "influx_application.h"

#include "core/platform/platform.h"
#include "core/platform/window.h"
#include "core/singleton.h"

// assets
#include "influx_assets.h"

namespace influx::application
{
	class content_cache final
	{
	public:
		content_cache(const string& resource_dir);

	private:
		map<string, assets::scene_data> m_scenes;
		map<string, assets::image_data> m_images;
		map<string, assets::shader_data> m_shaders;
	};

	class application final
		: public singleton<application>
	{
	public:
		void run(const run_args& args);

		void request_quit();

		string get_resource_directory() const;
		run_args get_run_arguments() const;

		platform::window_handle get_window_handle() const;
		platform::instance_handle get_instance_handle() const;

		static bool is_quit_requested();
		static bool is_vsync();
		static bool is_editor_enabled();
		static bool is_game_enabled();
		static bool is_scene_render_enabled();
		static bool is_commandlet();

	private:
		platform::window_handle m_windowhandle = nullptr;
		platform::instance_handle m_instancehandle = nullptr;

		uint64 m_frame = 0u;
		std::atomic_bool m_is_quit_requested = false;

		run_args m_run_args{};

		content_cache* mp_content_cache;

		void process_run_args(const run_args& args);
	};
}


