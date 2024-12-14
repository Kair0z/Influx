#pragma once

// influx::engine
#include "influx_engine/module/module.h"

namespace influx::engine
{
	struct update_context;

	class INFLUX_ENGINE_API game_module : public base_module
	{
	public:
		virtual ~game_module() = default;

		static void create_gameobject();

		// -- deriveable interface
		virtual void on_config(app_config&, game_config&);
		virtual void on_start();
		virtual void on_update(const update_context& ctx);
		virtual void on_end();
		virtual void on_cleanup();

		const game_config& get_config() const;

		// don't touch this
		void update(const update_context&);

	private:
		game_config m_config{};
	};
}