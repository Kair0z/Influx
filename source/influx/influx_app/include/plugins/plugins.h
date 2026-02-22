#pragma once

#include "core/string.h"

#if _DLL
#define INFLUX_APP_API __declspec(dllexport)
#else
#define INFLUX_APP_API __declspec(dllimport)
#endif

namespace influx::app
{
	class plugin
	{
	public:
		class event final
		{

		};

		class editor_interface final
		{
		public:
			
		};

		class window_interface final
		{
		public:

		};

		class console_interface final
		{
		public:

		};

		class command_interface final
		{
		public:
			INFLUX_APP_API void push();
		};

		class app_interface final
		{
		public:
			app_interface(
				console_interface& console,
				window_interface& window,
				command_interface& commands)
				: m_console{ console }, m_window{ window }, m_commands{ commands } {
			}

			command_interface& m_commands;
			console_interface& m_console;
			window_interface& m_window;
			bool m_is_active = false;
		};

	public:
		virtual void tick(app_interface& app) { };
		virtual void tick_imgui() {};
	};
	
}