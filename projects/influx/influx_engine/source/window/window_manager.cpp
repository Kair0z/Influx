#include "engine_pch.h"
#include "window_manager.h"

#include "influx_platform/window.h"

namespace influx::engine
{
	window_manager::window_manager()
	{	
		
	}

	result<window_manager::window_id> window_manager::spawn(const platform::window_desc& desc)
	{
		platform::window* new_window = platform::window::create(desc);
		new_window->set_event_callback([this](const platform::window_event& ev)
		{
			// on_window_event(ev);
		});

		// find an invalid removed slot:
		for (uint32 i = 0u; i < m_windows.size(); ++i)
		{
			if (!is_valid(i))
			{
				m_windows[i] = new_window;
				return i;
			}
		}

		// create new
		uint32 new_id = static_cast<uint32>(m_windows.size());
		m_windows.push_back(new_window);
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

	result<> window_manager::destroy(window_id id)
	{
		if (!is_valid(id)) return e_result::error;

		// 'destroy'
		// m_windows[id]
		m_windows[id] = nullptr;

		return {};
	}

	platform::window& window_manager::get_window(window_id id)
	{
		return *m_windows[id];
	}

	const platform::window& window_manager::get_window(window_id id) const
	{
		return *m_windows[id];
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
		return id < m_windows.size() && m_windows[id] != nullptr;
	}
}