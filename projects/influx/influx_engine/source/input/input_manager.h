#pragma once

// influx::core
#include "core/container/map.h"

// influx::input
#include "influx_input.h"

namespace influx::engine
{
	// collects influx_input events and dispatches them based on engine layers (editor / game / ...)
	class input_manager final
	{
	public:
		input_manager();
		~input_manager();

		void tick();
		void flush();
		void push_window_event(const platform::window_event&);

	private:
		void on_keydown(input::e_key);
		void on_keyup(input::e_key);
		void on_ascii_down(char);
		void on_ascii_up(char);
		void on_mouse_move(const input::mouse_position& position);
		void on_mouse_down(input::e_mouse_button button, const input::mouse_position& position);
		void on_mouse_up(input::e_mouse_button button, const input::mouse_position& position);

		template <typename _t>
		struct lock_queue
		{
			void push(const _t& val)
			{
				m_lock.lock();
				m_data.push(val);
				m_lock.unlock();
			}

			template <typename _readfunc>
			void read(_readfunc&& func) const
			{
				m_data.read(func);
			}

			void clear()
			{
				m_lock.lock();
				m_data.clear();
				m_lock.unlock();
			}

			queue<_t> get_copy()
			{
				return m_data;
			}

			queue<_t> m_data{};
			std::mutex m_lock;
		};
		lock_queue<input::e_key> m_deferred_keydowns{};
		lock_queue<input::e_key> m_deferred_keyups{};
		lock_queue<char> m_deferred_ascii_downs{};
		lock_queue<char> m_deferred_ascii_ups{};
		lock_queue<input::mouse_position> m_deferred_mousemoves{};
		lock_queue<std::pair<input::e_mouse_button, input::mouse_position>> m_deferred_mousedowns{};
		lock_queue<std::pair<input::e_mouse_button, input::mouse_position>> m_deferred_mouseups{};

		math::float2 m_mousepos = {};
		math::float2 m_mousedelta = {};
	};
}