#include "influx_app.h"

// STL
#include <iostream>

// influx::platform
#include "influx_platform/window.h"

namespace influx
{
	app::app(e_component_flags flags)
		: m_flags{flags}
	{
		
	}

	app::result<> app::run_impl()
	{
		if (has_console())
		{
			std::cout << "[influx app] Hello World! \n";
		}

		platform::window* window = nullptr;
		if (has_window())
		{
			window_settings settings = get_settings<e_settings::window>();
			platform::window_desc desc{};
			desc.m_dimensions = settings.m_dimensions;
			desc.m_name = settings.m_title;
			window = platform::window::create(desc);
		}

		while (!m_is_quit_requested)
		{
			// apply settings
			if (window)
			{
				window_settings settings = get_settings<e_settings::window>();
				window->set_dimensions(settings.m_dimensions);
			}
			if (has_console())
			{
				console_settings settings = get_settings<e_settings::console>();
			}

			// poll input
			if (window)
			{
				window->poll_events(m_is_quit_requested);
			}

			m_is_running = true;
		}
		
		m_is_running = false;
		return {};
	}

	bool app::has_console() const
	{
		return has_flag(m_flags, e_component_flags::console);
	}

	bool app::has_window() const
	{
		return has_flag(m_flags, e_component_flags::window);
	}

	bool app::is_running() const
	{
		return m_is_running;
	}

	void app::quit()
	{
		m_is_quit_requested = true;
	}

	app::result<> app::run(e_runmode mode)
	{
		if (mode == e_runmode::run_here)
		{
			return run_impl();
		}
		else
		{
			m_thread = thread([this]()
				{
					this->run_impl();
				});
			return{};
		}
	}

	app::~app()
	{
		if (m_is_running && !m_is_quit_requested)
		{
			quit();
		}

		if (m_thread.joinable())
			m_thread.join();
	}
}

