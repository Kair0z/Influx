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

// STL
#include <tuple>

namespace influx::app
{
	class plugin_manager;
	class window_manager;
	class console_manager;
	class command_manager;
	class editor_manager;
	class file_manager;
	class render_manager;

	class app final
	{
	public:
		enum class component
		{
			window = 0,
			console = 1,
			plugins = 2,
			editor = 3,
			num
		};

		template <typename _t = bool>
		using result = result<_t, const char*>;

		enum class component_flags : uint8
		{
			none		= 0,
			console		= 1 << static_cast<uint32>(component::console),
			window		= 1 << static_cast<uint32>(component::window),
			plugins		= 1 << static_cast<uint32>(component::plugins),
			editor		= 1 << static_cast<uint32>(component::editor),
			all			= console | window | plugins | editor
		};
		static component_flags make_flag(const component comp)
		{
			return (component_flags)(1u << static_cast<uint32>(comp));
		}

		INFLUX_APP_API app(component_flags components = component_flags::none);

		INFLUX_APP_API bool is_enabled(component comp) const;
		INFLUX_APP_API bool is_initialized(component comp) const;
		INFLUX_APP_API bool is_running() const;
		INFLUX_APP_API void quit();
		INFLUX_APP_API ~app();

		struct run_args final
		{
			int m_argc;
			char** m_argv;
			bool m_run_async = false;
		};
		INFLUX_APP_API app::result<> run(const run_args& args);

		INFLUX_APP_API void set_enabled(component_flags components);
		
		struct window_settings final
		{

		};
		struct console_settings final
		{

		};
		struct plugin_settings final
		{

		};

	private:
		component_flags		m_active_components = component_flags::none;
		thread				m_thread;
		bool				m_is_running = false;
		bool				m_is_quit_requested = false;
		
		plugin_manager*		m_plugin_man = nullptr;
		window_manager*		m_window_man = nullptr;
		console_manager*	m_console_man = nullptr;
		command_manager*	m_command_man = nullptr;
		file_manager*		m_file_man = nullptr;
		editor_manager*		m_editor_man = nullptr;
		render_manager*		m_render_man = nullptr;

		void create_and_destroy_components();
		void create_or_destroy(component comp, bool create);

		result<> run_impl();
	};
}
ENABLE_ENUM_BIT_OPERATORS(influx::app::app::component_flags);