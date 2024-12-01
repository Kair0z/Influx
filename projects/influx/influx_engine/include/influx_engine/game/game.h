#pragma once

// influx::engine
#include "influx_engine/engine/layergraph.h"
#include "influx_engine/engine/config.h"
#include "influx_engine/module/module.h"

namespace influx::engine
{
	struct update_context;

	class INFLUX_ENGINE_API game_module : public base_module
	{
	public:
		virtual ~game_module() = default;

		// -- layergraph
		template <class _ltype, class ..._args>
		_ltype* create_rootlayer(_args&&... args)
		{
			return m_layergraph.create_layer<_ltype, _args...>(nullptr, args...);
		}

		// -- deriveable interface
		virtual void on_config(app_config&, game_config&);
		virtual void on_start();
		virtual void on_update(const update_context& ctx);
		virtual void on_cleanup();

		const game_config& get_config() const;

		// don't touch this
		void update(const update_context&);

	private:
		game_config m_config{};
		layergraph m_layergraph{};
	};
}