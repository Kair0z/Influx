#include "engine_pch.h"
#include "window_manager.h"

// influx::engine
#include "influx_platform/window.h"
#include "input/input_manager.h"
#include "influx_platform/monitor.h"

namespace influx::engine
{
	window_manager::window_manager()
	{	
		m_monitors = platform::monitor::query_monitors();
	}

	void window_manager::on_window_event(const platform::window_event& ev)
	{
		platform::window* owner = ev.m_window;
		input_manager& inputman = get_engine()->get_input();
		result<window_id> id = get_window_id(owner);
		if (id.is_success())
		{
			inputman.push_window_event(ev);
		}
	}

	result<window_manager::window_id> window_manager::spawn(const platform::window_desc& desc)
	{
		platform::window* new_window = platform::window::create(desc);
		new_window->set_event_callback([this](const platform::window_event& ev)
		{
			on_window_event(ev);
		});

		// find an invalid removed slot:
		for (uint32 i = 0u; i < m_windows.size(); ++i)
		{
			if (!is_valid(i))
			{
				m_windows[i].m_window = new_window;
				return i;
			}
		}

		// create new
		uint32 new_id = static_cast<uint32>(m_windows.size());
		m_windows.push_back({.m_state = e_window_state::active, .m_window = new_window});
		return new_id;
	}

	window_manager::poll_result window_manager::poll(window_id id)
	{
		poll_result result{};
		if (is_valid(id) == false) { return result; }

		get_window(id).poll_events(result.m_is_quited);

		return result;
	}

	window_manager::poll_result window_manager::poll_main()
	{
		return poll(m_main_window_id);
	}

	window_manager::poll_result window_manager::poll_all()
	{
		poll_result result{};

		for (uint64 i = 0u; i < m_windows.size(); ++i)
		{
			poll_result this_result = poll(static_cast<uint32>(i));
			result.m_is_quited |= this_result.m_is_quited;
		}

		return result;
	}

	result<> window_manager::destroy(window_id id)
	{
		if (!is_valid(id))
			return result<>::make_error("error: invalid id!");

		// 'destroy'
		delete m_windows[id].m_window;
		m_windows[id].m_window = nullptr;

		return {};
	}

	platform::window& window_manager::get_window(window_id id)
	{
		return *m_windows[id].m_window;
	}

	const platform::window& window_manager::get_window(window_id id) const
	{
		return *m_windows[id].m_window;
	}

	platform::window& window_manager::get_main_window()
	{
		return get_window(m_main_window_id);
	}

	const platform::window& window_manager::get_main_window() const
	{
		return get_window(m_main_window_id);
	}

	uint32 window_manager::get_num_active_windows() const
	{
		return static_cast<uint32>(m_windows.size());
	}

	bool window_manager::is_valid(window_id id) const
	{
		return id < m_windows.size() && m_windows[id].m_window != nullptr;
	}

	bool window_manager::is_main(window_id id) const
	{
		return id == m_main_window_id;
	}

	bool window_manager::is_active(window_id id) const
	{
		return m_windows[id].m_state == e_window_state::active;
	}

	result<window_manager::window_id> window_manager::get_window_id(platform::window* window) const
	{
		using result_type = result<window_manager::window_id>;

		auto found = std::find_if(m_windows.cbegin(), m_windows.cend(), 
		[window](const window_manager::window& wind)
		{
			return wind.m_window->get_platform_handle() == window->get_platform_handle();
		});

		if (found != m_windows.cend())
		{
			return static_cast<uint32>(found - m_windows.cbegin());
		}

		return result_type::make_error("error: could not find the window id of this window!");
	}
	result<window_manager::window_id> window_manager::get_main_id() const
	{
		return m_main_window_id;
	}
}