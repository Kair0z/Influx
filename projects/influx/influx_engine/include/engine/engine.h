#pragma once

// influx::core
#include "core/string.h"
#include "core/math/vector.h"
#include "core/file.h"
#include "core/time.h"

// influx::engine
#include "game/game.h"
#include "editor/editor.h"
#include "config.h"

namespace influx::engine
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

	class game_module
	{
	public:
		struct ctx_update;

		virtual void on_config(app_config&, game_config&);
		virtual void on_start();
		virtual void on_level_loaded();
		virtual void on_update(const ctx_update& ctx);
		virtual void on_cleanup();

		void load_level(level* level);
		void load_level(const string& levelname);
		level const* get_current_level() const;
		const game_config& get_config() const;

		virtual ~game_module() = default;

	public:
		struct ctx_update final
		{
			frame_time m_frametime;

			ctx_update() = default;
			ctx_update(const frame_time& frtime);
		};

	private:
		game_config m_config;
	};

	class editor_module
	{
	public:
		virtual void on_config(app_config&, editor_config&);
		virtual void on_imgui();
		virtual void on_cleanup();

		virtual ~editor_module() = default;
	};
}

namespace influx::engine::detail
{
	extern game_module* create_game();
	extern editor_module* create_editor();

#define influx_engine_game(x) \
	influx::engine::game_module* influx::engine::detail::create_game() { return new x(); } \
	influx::engine::editor_module* influx::engine::detail::create_editor() { return nullptr; } 

#define influx_engine_editor(x) \
	influx::engine::editor_module* influx::engine::detail::create_editor() { return new x(); } \
	influx::engine::game_module* influx::engine::detail::create_game() { return nullptr; } 
}

