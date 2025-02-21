#include "engine_pch.h"
#include "input_manager.h"

namespace influx::engine
{
	input_manager::input_manager()
	{
		influx::input::init();

		input::subscribe_keydown([this](input::e_key key) { on_keydown(key); });
		input::subscribe_keyup([this](input::e_key key) { on_keyup(key); });
		input::subscribe_asciidown([this](char ascii) { on_ascii_down(ascii); });
		input::subscribe_asciiup([this](char ascii) { on_ascii_up(ascii); });
		input::subscribe_mousemove([this](const input::mouse_position& position)
		{
			on_mouse_move(position);
		});
		input::subscribe_mousedown([this](input::e_mouse_button button, const input::mouse_position& position)
		{
			on_mouse_down(button, position);
		});
		input::subscribe_mouseup([this](input::e_mouse_button button, const input::mouse_position& position)
		{
			on_mouse_up(button, position);
		});
	}

	input_manager::~input_manager()
	{
		flush();
	}

	void input_manager::tick()
	{
		// service input queue
		input::service_args args{};
		args.m_max_events_to_service = 64u;
		input::service(args);
	}

	void input_manager::flush()
	{ 
		m_deferred_keydowns.clear();
		m_deferred_keyups.clear();
		m_deferred_ascii_downs.clear();
		m_deferred_ascii_ups.clear();
		m_deferred_mousemoves.clear();
		m_deferred_mousedowns.clear();
		m_deferred_mouseups.clear();
	}

	void input_manager::push_window_event(const platform::window_event& ev)
	{
		input::push_window_event(ev);
	}

	void input_manager::on_keydown(input::e_key)
	{

	}

	void input_manager::on_keyup(input::e_key)
	{

	}

	void input_manager::on_ascii_down(char)
	{

	}

	void input_manager::on_ascii_up(char)
	{

	}

	void input_manager::on_mouse_move(const input::mouse_position& position)
	{

	}

	void input_manager::on_mouse_down(input::e_mouse_button button, const input::mouse_position& position)
	{

	}

	void input_manager::on_mouse_up(input::e_mouse_button button, const input::mouse_position& position)
	{

	}
}