#pragma once

#if _DLL
#define INFLUX_ENGINE_API __declspec(dllexport)
#else
#define INFLUX_ENGINE_API __declspec(dllimport)
#endif

// influx::core
#include "core/string.h"
#include "core/math/vector.h"
#include "core/file.h"

// influx::engine
#include "influx_engine/game/game.h"
#include "influx_engine/editor/editor.h"
#include "influx_engine/engine/config.h"
#include "influx_engine/engine/common.h"

namespace influx::engine
{
	class INFLUX_ENGINE_API base_module
	{
	public:
		virtual ~base_module() = default;

	protected:
		base_module() = default;
	};

	class INFLUX_ENGINE_API game_module : public base_module
	{
	public:
		struct ctx_update final
		{
			frame_time m_frametime;

			ctx_update() = default;
			ctx_update(const frame_time& frtime);
		};

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

	private:
		game_config m_config;
	};

	class INFLUX_ENGINE_API editor_module : public base_module
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
#define influx_engine_game(x) \
	influx::engine::base_module* create_module() { return new x(); } \

#define influx_engine_editor(x) \
	influx::engine::base_module* create_module() { return new x(); } \

	bool INFLUX_ENGINE_API run_engine(base_module* mod);
}

