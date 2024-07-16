#pragma once

#include "influx_application.h"

#include "core/platform/platform.h"
#include "core/platform/window.h"
#include "core/singleton.h"

// assets
#include "influx_assets.h"

namespace influx::renderer
{
	struct scene;
}

namespace influx::application
{
	struct frame_time final
	{
		float m_delta_seconds;
		float m_time_seconds;
	};

	class content_cache final
	{
	public:
		content_cache(const string& resource_dir);

		const map<string, assets::scene_data>& get_scenes() const;
		const map<string, assets::image_data>& get_images() const;
		const map<string, assets::shader_data>& get_shaders() const;

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
		string get_assets_directory() const;
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
		void load_render_assets();
		void update(renderer::scene& render_scene, const frame_time& time);

		platform::window_handle m_windowhandle = nullptr;
		platform::instance_handle m_instancehandle = nullptr;

		uint64 m_frame = 0u;
		std::atomic_bool m_is_quit_requested = false;

		run_args m_run_args{};
		string m_resource_dir{};
		string m_asset_dir{};
		bool m_staged{};

		content_cache* mp_content_cache;

		void process_run_args(const run_args& args);
	};
}


