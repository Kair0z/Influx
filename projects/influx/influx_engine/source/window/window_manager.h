#pragma once
#include "core/result.h"
#include "core/container/map.h"
#include "core/string.h"

// influx::platform
#include "influx_platform/monitor.h"
namespace influx::platform
{
	class window;
	class window_event;
	struct window_desc;
}

namespace influx::engine
{
	enum class e_window_state : uint8
	{
		active,
		inactive,
		count
	};

	class window_manager final
	{
	public:
		using window_id = uint32;
		static constexpr window_id k_invalid_id = (uint32)-1;

		window_manager();
		~window_manager() = default;

		result<window_id> spawn(const platform::window_desc& desc);

		struct poll_result final
		{
			bool m_is_quited = false;
		};
		poll_result poll(window_id id);
		poll_result poll_main();
		poll_result poll_all();

		result<> destroy(window_id id);

		platform::window& get_window(window_id id);
		const platform::window& get_window(window_id id) const;

		platform::window& get_main_window();
		const platform::window& get_main_window() const;

		uint32 get_num_active_windows() const;

		bool is_valid(window_id id) const;
		bool is_main(window_id id) const;
		bool is_active(window_id id) const;
		result<window_id> get_window_id(platform::window*) const;
		result<window_id> get_main_id() const;
	private:
		void on_window_event(const platform::window_event& ev);

		struct window final
		{
			e_window_state m_state;
			platform::window* m_window;
		};
		vector<window> m_windows{};
		vector<platform::monitor> m_monitors{};
		window_id m_main_window_id = 0u;
	};
}