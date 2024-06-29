#pragma once
#include "core/platform/window.h"

namespace influx::platform
{
	window_handle get_current_window()
	{
		return nullptr;
	}

	bool is_window_valid(window_handle handle)
	{
		return false;
	}

	window_handle create_window(const create_window_args& args, bool make_open)
	{
		return nullptr;
	}

	void destroy_window(const window_handle handle)
	{

	}

	bool set_window_visible(const window_handle window, const e_window_visibility command)
	{
		return false;
	}

	template <typename _t>
	math::rect<_t> get_windowrect_full(const window_handle window)
	{

	}

	template <typename _t>
	math::rect<_t> get_windowrect_client(const window_handle window)
	{

	}

	bool is_window_visible(const window_handle window)
	{
		return false;
	}

	void messagebox(e_messagebox type, const string& caption, const string& message, const window_handle handle)
	{

	}
}