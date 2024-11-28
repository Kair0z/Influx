#pragma once

// influx::engine
#include "influx_engine/world/world.h"
#include "influx_engine/engine/component.h"

namespace influx::engine
{
	class layergraph;
	class gameobject;

	// -- recognize this as 'scene', just more hierarchical and abstract
	// -- layergraph manages layer hierarchy tick schedule
	// -- world manages objects like gameobject & components
	class layer
	{
	public:
		INFLUX_ENGINE_API gameobject* create();

		template <class _ctype, class ..._args>
		inline _ctype* create_component(uint32 object_id, _args&&... args)
		{
			world* current_world = detail::get_world();
			return current_world->create_component<_ctype>(object_id, args...);
		}

		// -- deriveable interface
		virtual void on_start() {}
		virtual void on_update(const update_context&) {}
		virtual void on_exit() {}

	private:
		layergraph* m_owner;
		friend class layergraph;
		uint64 m_frame_counter{};
	};
}