#include "input_pch.h"

#include "core/singleton.h"

// events
#include "influx_events.h"

#include "core/platform/win32/win32_window.h"

namespace influx::input
{
	class global_state final : public singleton<global_state>
	{
	public:
		static events::event_queue*& get_queue()
		{
			return get_instance().mp_event_queue;
		}

	private:
		events::event_queue* mp_event_queue;
	};

	void push(key_event* key_ev)
	{
		events::event ev { key_ev };

		global_state::get_queue()->push(ev);
	}

	void init(const init_args& args)
	{
		// create the events::queue
		global_state::get_queue() = new events::event_queue();
	}

	inline static key_event::e_type translate(platform::window_event::type plat_type)
	{
		switch (plat_type)
		{
		case platform::window_event::type::keydown: return key_event::e_type::keydown;
		case platform::window_event::type::keyup: return key_event::e_type::keyup;
		}

		return key_event::e_type::count;
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
		}

		return e_key::count;
	}

	void push_window_event(const platform::window_event& platform_ev)
	{
		// filter key-events only
		void* data = nullptr;
		key_event* new_key_ev = nullptr;
		mouse_event* new_mouse_ev = nullptr;
		switch (platform_ev.m_type)
		{
		case platform::window_event::type::keydown:
			new_key_ev = new key_event();
			new_key_ev->m_type = key_event::e_type::keydown;
			data = new_key_ev;
			break;

		case platform::window_event::type::keyup:
			new_key_ev = new key_event();
			new_key_ev->m_type = key_event::e_type::keyup;
			data = new_key_ev;
			break;

		case platform::window_event::type::wheel:
			new_mouse_ev = new mouse_event();
			new_mouse_ev->m_wheel_delta = platform_ev.parse_wheel_delta();
			data = new_mouse_ev;
			break;
		}

		// parse further key_event data
		if (new_key_ev)
		{
			platform::window_event::key_type key_type = platform_ev.parse_key_type();
			new_key_ev->m_key = translate(key_type);

			// store an optional ascii char
			switch (key_type)
			{
			case platform::window_event::key_type::ascii_ch:
			case platform::window_event::key_type::ascii_num:
				new_key_ev->m_ascii_char = platform_ev.parse_ascii();
				break;
			}
		}

		// push into the queue
		if (data != nullptr)
		{
			global_state::get_queue()->push(events::event{ data });
		}
	}

	void subscribe(const key_callback& callback)
	{
		auto this_callback = [callback](const events::event& ev)
		{
			key_event* key_ev = reinterpret_cast<key_event*>(ev.get_data());
			callback(*key_ev);
		};

		global_state::get_queue()->subscribe(this_callback);
	}

	void subscribe(const mouse_callback& callback)
	{
		auto this_callback = [callback](const events::event& ev)
		{
			mouse_event* mouse_ev = reinterpret_cast<mouse_event*>(ev.get_data());
			callback(*mouse_ev);
		};

		global_state::get_queue()->subscribe(this_callback);
	}

	void cleanup()
	{
		delete global_state::get_queue();
	}

	void service()
	{
		global_state::get_queue()->service();
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