#pragma once
#include "core/result.h"
#include "core/container/map.h"
#include "core/string.h"

// influx::platform
namespace influx::platform
{
	class window;
	class window_event;
}

namespace influx::engine
{
	class window_manager final
	{
		using window_id = uint32;
	public:
		window_manager();
		~window_manager() = default;

		result<window_id> spawn(const platform::window_desc& desc);

		struct poll_result final
		{
			bool m_is_quited = false;
		};
		poll_result poll(window_id id);

		poll_result poll_main();

		result<> destroy(window_id id);

		platform::window& get_window(window_id id);
		const platform::window& get_window(window_id id) const;

		platform::window& get_main_window();
		const platform::window& get_main_window() const;

		uint32 get_num_active_windows() const;

		bool is_valid(window_id id) const;

	private:
		vector<platform::window*> m_windows{};
		window_id m_main_window_id = 0u;
	};
}