#pragma once
#include "influx_platform/window.h"

namespace influx::platform
{
	class win32_window final : public window
	{
	public:
		win32_window(const window_desc& desc);

		virtual void set_visibility(e_visibility) override;

		virtual void poll_events(bool& is_quit) const override;

		virtual window_handle get_platform_handle() const override;

		virtual rect get_rect_full() const override;

		virtual rect get_rect_client() const override;

		virtual void set_event_callback(const event_callback&) override;

		~win32_window();

	private:
		window_handle m_handle;
		list<event_callback> m_event_callbacks;

	public:
		static uint64 window_proc(window_handle handle, uint32 message, uint64 wParam, uint64 lParam);
	};
}