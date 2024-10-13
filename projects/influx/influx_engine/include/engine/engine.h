#pragma once

// influx::core
#include "core/string.h"
#include "core/math/vector.h"
#include "core/file.h"

namespace influx::engine
{
	class game_module
	{
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

		virtual void on_config(config&);
		virtual void on_start();
		virtual void on_update();
		virtual void on_cleanup();

		virtual ~game_module() = default;
	};

	class editor_module
	{
	public:
		virtual void on_imgui();

		virtual ~editor_module() = default;
	};

	namespace detail
	{
		extern game_module* create_game();
		extern editor_module* create_editor();
	}
}

#define influx_engine_game(x) \
	influx::engine::game_module* influx::engine::detail::create_game() { return new x(); } 

#define influx_engine_editor(x) \
	influx::engine::editor_module* influx::engine::detail::create_editor() { return new x(); }
