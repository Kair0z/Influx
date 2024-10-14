#pragma once

// influx::core
#include "core/string.h"
#include "core/math/vector.h"
#include "core/file.h"
#include "core/time.h"

// influx::engine
#include "game/game.h"
#include "editor/editor.h"

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
		struct config;
		struct ctx_update;

		virtual void on_config(config&);
		virtual void on_start();
		virtual void on_level_loaded();
		virtual void on_update(const ctx_update& ctx);
		virtual void on_cleanup();

		void load_level(level* level);
		void load_level(const string& levelname);
		level const* get_current_level() const;
		const config& get_config() const;

		virtual ~game_module() = default;

	public:
		struct config final
		{
			using self = config;
			self& set_gamefile(const string& file);
			self& set_window_dim(const math::vectoru2& dim);
			self& set_name(const string& name);

			string m_gamefile_path;
			string m_gamename;
			math::vectoru2 m_window_dimensions;

			// setup by the engine
			file m_file_influx_root;
			file m_file_influx_assets;
			file m_file_influx_staged;
			file m_file_influx_resources;
		};

		struct ctx_update final
		{
			frame_time m_frametime;

			ctx_update() = default;
			ctx_update(const frame_time& frtime);
		};

	private:
		config m_config;
	};

	class editor_module
	{
	public:
		virtual void on_imgui();

		virtual ~editor_module() = default;
	};
}

namespace influx::engine::detail
{
	extern game_module* create_game();
	extern editor_module* create_editor();

#define influx_engine_game(x) \
	influx::engine::game_module* influx::engine::detail::create_game() { return new x(); } 

#define influx_engine_editor(x) \
	influx::engine::editor_module* influx::engine::detail::create_editor() { return new x(); }
}

