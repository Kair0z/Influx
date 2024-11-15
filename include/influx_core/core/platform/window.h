#pragma once

#if 0
// influx::core
#include "core/basetypes.h"
#include "core/math/vector.h"
#include "core/geometry/rect.h"
#include "core/string.h"
#include "core/function.h"

// influx::core::platform
#include "core/platform/platform.h"

namespace influx::platform
{
	window_handle get_current_window();

	bool is_window_valid(window_handle handle);
	
	struct create_window_args final
	{
		create_window_args() = default;
		create_window_args(const math::vectoru2& dimensions, const string& name, const window_proc_callback& proc_clb)
			: m_width{ dimensions.x }
			, m_height{ dimensions.y }
			, m_name{ name }
			, m_proc_callback{ proc_clb } {}

		uint32 m_width;
		uint32 m_height;
		string m_name;
		window_proc_callback m_proc_callback;
	};

	window_handle create_window(const create_window_args& args);

	void add_window_proc(const window_handle handle, const window_proc_callback& callback);

	void destroy_window(const window_handle handle);

	bool set_window_visible(const window_handle window, const e_window_visibility command);

	template <typename _t>
	math::rect<_t> get_windowrect_full(const window_handle window);

	template <typename _t>
	math::rect<_t> get_windowrect_client(const window_handle window);

	bool is_window_visible(const window_handle window);

	void messagebox(e_messagebox type, const string& caption, const string& message, const window_handle handle = get_current_window());
}
#endif