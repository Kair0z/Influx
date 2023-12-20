#pragma once

#include "influx_application.h"
#include "konstants.h"

#include "core/platform/platform.h"
#include "core/singleton/singleton.h"

namespace influx::application
{
	struct per_frame_stats final
	{
		float m_ms_total = 0.0f;	// total ms frame
		float m_pc_sync = 0.0f;		// percentage of total ms spent on syncing

		per_frame_stats& operator+=(const per_frame_stats& other)
		{
			m_pc_sync += other.m_pc_sync;
			m_ms_total += other.m_ms_total;
			return *this;
		}
		per_frame_stats& operator/=(const float& div)
		{
			m_pc_sync /= div;
			m_ms_total /= div;
			return *this;
		}
	};

	class gamethread;
	class renderthread;
	class dedicated_thread;
	class layer_stack;
	class rendersync;

	class application final
		: public singleton<application>
	{
	public:
		void run(const run_args& args, base* sub_module);
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

		rendersync& get_render_sync();

	private:
		void main_init();
		void main_tick();
		void main_cleanup();
		uint64 m_mainthread_frame = 0u;

		platform::window_handle m_windowhandle = nullptr;
		platform::instance_handle m_instancehandle = nullptr;
		std::atomic_bool m_is_quit_requested = false;

		vector<dedicated_thread*> m_dedicated_threads{};
		gamethread* mp_gamethread = nullptr;
		renderthread* mp_renderthread = nullptr;
		layer_stack* mp_layerstack = nullptr;
		rendersync* mp_rendersync = nullptr;

		run_args m_run_args{};
		base* mp_base_application = nullptr;
	};
}


