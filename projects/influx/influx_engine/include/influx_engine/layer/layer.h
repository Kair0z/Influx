#pragma once

// influx::engine
#include "influx_engine/world/world.h"
#include "influx_engine/component.h"

// influx::input
#include "influx_input.h"

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
		gameobject* create();

		template <class _ctype, class ..._args>
		inline _ctype* create_component(uint32 object_id, _args&&... args)
		{
			return nullptr;
		}

		// -- deriveable interface
		virtual void on_start() {}
		virtual void on_update(const update_context&) {}
		virtual void on_exit() {}

		virtual void on_keydown(input::e_key) {}
		virtual void on_keyup(input::e_key) {}
		virtual void on_ascii_down(char) {}
		virtual void on_ascii_up(char) {}
		virtual void on_mouse_move(const input::mouse_position& position) {}
		virtual void on_mouse_down(input::e_mouse_button button, const input::mouse_position& position) {}
		virtual void on_mouse_up(input::e_mouse_button button, const input::mouse_position& position) {}

	private:
		layergraph* m_owner;
		friend class layergraph;
		uint64 m_frame_counter{};
	};
}