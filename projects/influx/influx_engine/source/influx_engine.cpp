#include "engine_pch.h"

// influx::platform
#include "platform.h"

// influx::core
#include "core/time.h"
#include "core/log.h"

namespace influx::engine::detail
{
	struct frame_time final
	{
		float m_delta_seconds;
		float m_time_seconds;

		influx::time::point m_first_tick;
		influx::time::point m_last_tick;
		bool m_is_first_tick = true;

		inline void tick()
		{
			if (m_is_first_tick)
			{
				m_first_tick = influx::time::get_now();
				m_last_tick = m_first_tick;
				m_is_first_tick = false;
			}

			const float delta_seconds = influx::time::get_ms_since<float>(m_last_tick) * 0.001f;
			m_delta_seconds = delta_seconds;
			m_time_seconds += delta_seconds;

			m_last_tick = influx::time::get_now();
		}
	};

	void setup_engineconfig(game_module::config& config)
	{
		// builds are ran in /influx/bin/[config]/influx_game/
		const string& root_influx = platform::platform::get_current_directory() + "/../../../";
		config.m_file_influx_root = root_influx;
		config.m_file_influx_resources = root_influx + "/resources/";
		config.m_file_influx_assets = root_influx + "/assets/";
		config.m_file_influx_staged = root_influx + "/staged/";
	}

	void run_game()
	{
		// create game object:
		game_module* new_game = create_game();

		// build game config:
		game_module::config game_config{};
		setup_engineconfig(game_config);
		new_game->on_config(game_config);

		// parse game config:
		platform::window_desc window_desc{};
		window_desc
			.set_dimensions(game_config.m_window_dimensions)
			.set_name(game_config.m_gamename);

		// make a window
		platform::window* game_window 
			= platform::window::create(window_desc);

		// some stack variables
		time::point initial_tick = time::get_now();
		time::point last_tick = initial_tick;
		frame_time frame_time{};

		// LOOP
		bool platform_quit_event = false;
		while (!platform_quit_event)
		{
			frame_time.tick();

			game_window->poll_events(platform_quit_event);
			platform_quit_event |= game_window->has_quit_request();
		}

		// destroy the window
		delete game_window;

		// cleanup game
		new_game->on_cleanup();
		delete new_game;
	}

	void run_editor()
	{

	}

	void run_engine()
	{
		// todo - for now just runs game...
		run_game();
	}
}