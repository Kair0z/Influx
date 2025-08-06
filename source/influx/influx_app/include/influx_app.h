#pragma once

#if _DLL
#define INFLUX_APP_API __declspec(dllexport)
#else
#define INFLUX_APP_API __declspec(dllimport)
#endif

// influx::core
#include "core/basetypes.h"
#include "core/enum.h"
#include "core/result.h"
#include "core/threading/thread.h"
#include "core/math/vector.h"

#include <tuple>

namespace influx
{
	class app final
	{
		
	public:
		enum class e_settings
		{
			window,
			console
		};

		struct window_settings final
		{
			string m_title = "influx app";
			math::uint2 m_dimensions = { 640u, 480u };
		};
		struct console_settings final
		{
			
		};
		template <e_settings _t>
		using settings_t = std::tuple_element_t<static_cast<uint32>(_t), std::tuple<
			window_settings,
			console_settings>>;
		
		template <typename _t = bool>
		using result = result<_t, const char*>;

		enum class e_component_flags : uint8
		{
			none		= 0,
			console		= 1 << 0,
			window		= 1 << 1
		};

		INFLUX_APP_API app(e_component_flags flags);

		INFLUX_APP_API bool has_console() const;

		INFLUX_APP_API bool has_window() const;

		INFLUX_APP_API bool is_running() const;

		INFLUX_APP_API void quit();

		INFLUX_APP_API ~app();

		enum class e_runmode
		{
			run_here,
			run_on_thread
		};

		INFLUX_APP_API app::result<> run(e_runmode mode);

		// settings
		template <e_settings _t>
		void set_settings(const settings_t<_t>& settings);

		template <e_settings _t>
		const settings_t<_t>& get_settings() const;

	private:
		settings_t<e_settings::window> m_window_settings;
		settings_t<e_settings::console> m_console_settings;

		e_component_flags m_flags = e_component_flags::none;
		thread m_thread;
		bool m_is_running = false;
		bool m_is_quit_requested = false;
		app::result<> run_impl();
	};

	template <app::e_settings _t>
	void app::set_settings(const app::settings_t<_t>& settings)
	{
		if constexpr (_t == e_settings::window) m_window_settings = settings;
		if constexpr (_t == e_settings::console) m_console_settings = settings;
	}

	template <app::e_settings _t>
	const app::settings_t<_t>& app::get_settings() const
	{
		if constexpr (_t == e_settings::window) return m_window_settings;
		if constexpr (_t == e_settings::console) return m_console_settings;
		return {};
	}
}
ENABLE_ENUM_BIT_OPERATORS(influx::app::e_component_flags);