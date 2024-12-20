#include "input_pch.h"

#include "core/singleton.h"
#include "core/events/event_queue.h"

// influx::platform
#include "influx_platform/window.h"

namespace influx::input
{
	using input_event_queue = events::event_queue<key_event, mouse_event>;
	using input_event = input_event_queue::my_event;

	class global_state final : public singleton<global_state>
	{
	public:
		static input_event_queue*& get_queue()
		{
			return get_instance().mp_event_queue;
		}

	private:
		input_event_queue* mp_event_queue;
	};

	template <typename _evtype, typename... _args>
	void push(_args&&... args)
	{
		global_state::get_queue()->push<key_event>(args);
	}

	void init(const init_args& args)
	{
		// create the events::queue
		global_state::get_queue() = new input_event_queue();
	}

	void subscribe_keydown(const function<void(e_key)>& callback)
	{
		subscribe([callback](const key_event& keyev)
		{
			if (callback && keyev.m_type == key_event::e_type::keydown)
				callback(keyev.m_key);
		});
	}

	void subscribe_keyup(const function<void(e_key)>& callback)
	{
		subscribe([callback](const key_event& keyev)
		{
			if (callback && keyev.m_type == key_event::e_type::keyup)
				callback(keyev.m_key);
		});
	}

	void subscribe_asciidown(const function<void(char)>& callback)
	{
		subscribe([callback](const key_event& keyev)
		{
			if (callback && keyev.is_ascii() && keyev.m_type == key_event::e_type::keydown)
				callback(keyev.m_ascii_char);
		});
	}

	void subscribe_asciiup(const function<void(char)>& callback)
	{
		subscribe([callback](const key_event& keyev)
		{
			if (callback && keyev.is_ascii() && keyev.m_type == key_event::e_type::keyup)
				callback(keyev.m_ascii_char);
		});
	}

	void subscribe_mousemove(const function<void(const mouse_position&)>& callback)
	{
		subscribe([callback](const mouse_event& ev)
		{
			if (callback && ev.m_type == mouse_event::type::move)
				callback(ev.m_position);
		});
	}

	void subscribe_mousedown(const function<void(e_mouse_button, const mouse_position&)>& callback)
	{
		subscribe([callback](const mouse_event& ev)
		{
			if (callback && ev.m_type == mouse_event::type::button_down)
				callback(ev.m_button, ev.m_position);
		});
	}

	void subscribe_mouseup(const function<void(e_mouse_button, const mouse_position&)>& callback)
	{
		subscribe([callback](const mouse_event& ev)
		{
			if (callback && ev.m_type == mouse_event::type::button_up)
				callback(ev.m_button, ev.m_position);
		});
	}

#pragma region translation
	inline static key_event::e_type translate_key(platform::window_event::type plat_type)
	{
		switch (plat_type)
		{
		case platform::window_event::type::keydown: return key_event::e_type::keydown;
		case platform::window_event::type::keyup: return key_event::e_type::keyup;
		}

		return key_event::e_type::count;
	}

	inline static mouse_event::type translate_mouse(platform::window_event::type plat_type)
	{
		switch (plat_type)
		{
			case platform::window_event::type::mouse_down: return mouse_event::type::button_down;
			case platform::window_event::type::mouse_up: return mouse_event::type::button_up;
			case platform::window_event::type::mouse_leave: return mouse_event::type::leave;
			case platform::window_event::type::mouse_move: return mouse_event::type::move;
			case platform::window_event::type::wheel: return mouse_event::type::scroll;
		}

		return mouse_event::type::count;
	}

	inline static e_key translate(platform::window_event::key_type plat_key)
	{
		switch (plat_key)
		{
		case platform::window_event::key_type::ascii_ch: return e_key::ascii_char;
		case platform::window_event::key_type::ascii_num: return e_key::ascii_num;
		case platform::window_event::key_type::up: return e_key::up;
		case platform::window_event::key_type::down: return e_key::down;
		case platform::window_event::key_type::right: return e_key::right;
		case platform::window_event::key_type::left: return e_key::left;
		case platform::window_event::key_type::lshift: return e_key::lshift;
		case platform::window_event::key_type::rshift: return e_key::rshift;
		case platform::window_event::key_type::lctrl: return e_key::lctrl;
		case platform::window_event::key_type::rctrl: return e_key::rctrl;
		case platform::window_event::key_type::lalt: return e_key::lalt;
		case platform::window_event::key_type::ralt: return e_key::ralt;
		case platform::window_event::key_type::space: return e_key::space;
		}

		return e_key::count;
	}

	inline static e_mouse_button translate(platform::window_event::mouse_button button)
	{
		switch (button)
		{
		case platform::window_event::mouse_button::left: return e_mouse_button::left;
		case platform::window_event::mouse_button::right: return e_mouse_button::right;
		case platform::window_event::mouse_button::middle: return e_mouse_button::middle;
		case platform::window_event::mouse_button::x: return e_mouse_button::x;
		}

		return e_mouse_button::count;
	}
#pragma endregion

	void push_window_event(const platform::window_event& platform_ev)
	{
		const bool is_key_event = platform_ev.is_key_event();
		const bool is_mouse_event = platform_ev.is_mouse_event();

		if (is_key_event)
		{
			key_event ev{};
			ev.m_type = translate_key(platform_ev.m_type);
			ev.m_key = translate(platform_ev.parse_key_type());
			ev.m_ascii_char = platform_ev.parse_ascii();
			global_state::get_queue()->push<key_event>(ev);
		}

		if (is_mouse_event)
		{
			mouse_event ev{};
			ev.m_type = translate_mouse(platform_ev.m_type);
			ev.m_position = { platform_ev.parse_position_window(), platform_ev.parse_position_screen() };
			ev.m_button = translate(platform_ev.parse_mouse_button());
			ev.m_wheel_delta = platform_ev.parse_wheel_delta();
			global_state::get_queue()->push<mouse_event>(ev);
		}
	}

	void push_external_event(const key_event& ev)
	{
		global_state::get_queue()->push<key_event>(ev);
	}

	void push_external_event(const mouse_event& ev)
	{
		global_state::get_queue()->push<mouse_event>(ev);
	}

	void subscribe(const key_callback& callback)
	{
		global_state::get_queue()->subscribe<key_event>([callback](const key_event& ev)
		{
			callback(ev);
		});
	}

	void subscribe(const mouse_callback& callback)
	{
		global_state::get_queue()->subscribe<mouse_event>([callback](const mouse_event& ev)
		{
			callback(ev);
		});
	}

	void cleanup()
	{
		delete global_state::get_queue();
	}

	void service(const service_args& args)
	{
		input_event_queue::process_args process_args{};
		process_args.m_max_num_events = args.m_max_events_to_service;
		global_state::get_queue()->process(process_args);
	}

	bool key_event::is_ascii() const
	{
		return m_key == e_key::ascii_char || m_key == e_key::ascii_num;
	}

	string key_event::to_string() const
	{
		switch (m_key)
		{
		case e_key::ascii_char:
		case e_key::ascii_num:
			return string(1, m_ascii_char);

		case e_key::left: return "left";
		case e_key::right: return "right";
		case e_key::down: return "down";
		case e_key::up: return "up";
		}

		return "unknown";
	}
}