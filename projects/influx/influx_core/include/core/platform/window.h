#pragma once
#include "core/platform/platform.h"

#include "core/basetypes.h"
#include "core/math/vector.h"
#include "core/geometry/rect.h"
#include "core/string.h"
#include "core/function.h"

namespace influx::platform
{
	using window_handle = void*;

	// window event callback types
	typedef void(*winev_callback)	();
	typedef void(*winev_mousepos)	(const float x, const float y);
	typedef void(*winev_mousebutton)(int button, bool isDown);
	typedef void(*winev_mousewheel)	(const float w_x, const float w_y);
	typedef void(*winev_focus)		(bool is_focussed);
	
	enum class e_windowevent : uint8
	{
		activate,
		quit,
		max,
		unknown = max
	};

	struct window_event final
	{
		enum class type : uint8
		{
			// input
			keydown,
			keyup,
			// general
			activate,
			quit,
			count
		} m_type;

		enum class key_type : uint8
		{
			left,
			right,
			up,
			down,
			home,
			end,
			insert,
			deleet,
			f2,
			unknown,
			count
		};

		key_type parse_key_type() const;

		uint64 m_wParam;
		uint64 m_lParam;
	};

	using window_proc_callback = function<void(const window_event& e)>;

	enum class e_window_visibility : uint8
	{
		minimized,
		showed,
		maximized,
		count
	};

	enum class e_messagebox : uint8
	{
		info,
		warning,
		error,
		count
	};

	window_handle get_current_window();

	bool is_window_valid(window_handle handle);

	// create a window
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

	void destroy_window(const window_handle handle);

	bool set_window_visible(const window_handle window, const e_window_visibility command);

	template <typename _t>
	math::rect<_t> get_windowrect_full(const window_handle window);

	template <typename _t>
	math::rect<_t> get_windowrect_client(const window_handle window);

	bool is_window_visible(const window_handle window);

	void messagebox(e_messagebox type, const string& caption, const string& message, const window_handle handle = get_current_window());
}