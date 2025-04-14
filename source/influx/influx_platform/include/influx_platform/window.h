#pragma once 
#include "platform.h"

// influx::core
#include "core/string.h"
#include "core/math/vector.h"
#include "core/geometry/rect.h"
#include "core/function.h"

namespace influx::platform
{
	class window;

	enum class e_messagebox : uint8
	{
		info,
		warning,
		error,
		count
	};

	class window_event final
	{
	public:
		enum class type : uint8
		{
			// input
			_key_begin,
			keydown,
			keyup,
			_key_end,

			// mouse
			_mouse_begin,
			wheel,
			mouse_move,
			mouse_leave,
			mouse_down,
			mouse_up,
			_mouse_end,

			// general
			activate,
			quit,
			close,
			move,
			size,
			mouse_activate,
			count
		};
		enum class mouse_button : uint8
		{
			left,
			right,
			middle,
			x,
			count
		};
		enum class key_type : uint8
		{
			left,right,up,down,
			home,end,insert,deleet,
			apostrophe,comma,minus,plus,period, 
			backslash,slash,semicolon,equal,lbracket,rbracket,
			f2,
			lshift, rshift, lctrl, rctrl, lalt, ralt,
			space, backspace, enter,
			ascii_num, // ascii number (0-9)
			ascii_ch, // ascii character (A-Z)
			unknown,
			count
		};

		key_type	INFLUX_PLATFORM_API	parse_key_type() const;
		char		INFLUX_PLATFORM_API	parse_ascii() const;
		float		INFLUX_PLATFORM_API parse_wheel_delta() const;
		math::vectorf2 INFLUX_PLATFORM_API parse_position_window() const;
		math::vectorf2 INFLUX_PLATFORM_API parse_position_screen() const;
		math::vectorf2 INFLUX_PLATFORM_API parse_position_window_normalized() const;
		math::vectorf2 INFLUX_PLATFORM_API parse_position_screen_normalized() const;
		mouse_button INFLUX_PLATFORM_API parse_mouse_button() const;
		bool INFLUX_PLATFORM_API is_mouse_event() const;
		bool INFLUX_PLATFORM_API is_key_event() const;
		bool INFLUX_PLATFORM_API is_wheel_event() const;

		uint32 m_mssg;
		uint64 m_wParam;
		uint64 m_lParam;
		window* m_window;
		type m_type;
	};

	struct window_style final
	{
		static const window_style& get_generic()
		{
			static window_style generic_style{};
			generic_style.set_generic_window(true);
			return generic_style;
		}

		INFLUX_PLATFORM_API void set_decoration(bool enabled);
		INFLUX_PLATFORM_API bool get_decoration() const;
		INFLUX_PLATFORM_API void set_taskicon_enabled(bool enabled);
		INFLUX_PLATFORM_API bool get_taskicon_enabled() const;
		INFLUX_PLATFORM_API void set_topmost(bool enabled);

		// WS_OVERLAPPEDWINDOW
		INFLUX_PLATFORM_API void set_generic_window(bool enabled);
		INFLUX_PLATFORM_API void set_exit_button(bool enabled);

		uint32 m_style;
		uint32 m_style_ext;
	};

	struct window_desc final
	{
		inline window_desc& set_dimensions(const math::vectoru2& dim)
		{
			m_dimensions = dim;
			return *this;
		}

		inline window_desc& set_position(const math::vectorf2& pos)
		{
			m_position = pos;
			return *this;
		}

		inline window_desc& set_name(const string& name)
		{
			m_name = name;
			return *this;
		}

		inline window_desc& set_style(const window_style& style)
		{
			m_style = style;
			return *this;
		}

		inline window_style& style() { return m_style; }

		math::vectorf2 m_position;
		math::vectoru2 m_dimensions;
		string m_name;
		window_style m_style = window_style::get_generic();
	};

	class window
	{
	public:
		INFLUX_PLATFORM_API static window* create(const window_desc& desc);
		INFLUX_PLATFORM_API static window* import_window(const window_handle& handle);

		// gets the main current window
		static window& get_current();

		enum class e_space : uint8
		{
			client,		// space of the client area
			full,		// space of the client + non-client area
			count
		};

		enum class e_visibility : uint8
		{
			minimized,
			showed,
			maximized,
			count
		};

		using event_callback = function<void(const window_event& e)>;

		virtual window_handle get_platform_handle() const { return nullptr; }

		INFLUX_PLATFORM_API
		virtual void poll_events(bool& is_quit) const { };

		INFLUX_PLATFORM_API
		virtual bool is_valid() const { return false; };

		INFLUX_PLATFORM_API
		virtual void set_event_callback(const event_callback&) { };

		virtual void set_visibility(e_visibility) { };

		virtual bool is_visible() const { return false; };

		virtual bool is_visible(e_visibility& out_vis) const { return false; };

		virtual void messagebox(e_messagebox type,
			const string& caption,
			const string& message) const { };

		using rect = math::rect<uint32>;

		INFLUX_PLATFORM_API
		virtual void set_dimensions(const math::vectoru2& new_dimensions) { }

		INFLUX_PLATFORM_API
		virtual math::vectoru2 get_dimensions(e_space) const { return {}; }

		INFLUX_PLATFORM_API
		virtual math::vectoru2 get_previous_dimensions(e_space) const { return {}; }

		INFLUX_PLATFORM_API
		virtual void set_position(const math::vectoru2& ) { }

		INFLUX_PLATFORM_API 
		virtual math::vectoru2 get_position() const { return {}; }

		INFLUX_PLATFORM_API
		virtual rect get_rect(e_space) const { return {}; };

		INFLUX_PLATFORM_API
		virtual void set_title(const string& new_title) {}

		INFLUX_PLATFORM_API
		virtual bool is_foreground() const { return false;}

		INFLUX_PLATFORM_API
		virtual void set_foreground() {}

		INFLUX_PLATFORM_API 
		virtual bool is_focus() const { return false; }

		INFLUX_PLATFORM_API
		virtual void set_focus() {}

		INFLUX_PLATFORM_API
		virtual bool is_minimized() const { return false; }

		INFLUX_PLATFORM_API
		virtual string get_title() const { return "";};

		INFLUX_PLATFORM_API
		virtual void set_alpha(float) { }

		INFLUX_PLATFORM_API
		virtual float get_alpha() const { return 1.0f; }

		INFLUX_PLATFORM_API
		virtual float get_dpi() const { return 1.0f; }

		INFLUX_PLATFORM_API
		virtual window_style get_style() const { return {}; }

		INFLUX_PLATFORM_API
		virtual void set_style(const window_style&) { }

		INFLUX_PLATFORM_API
		virtual void set_parent(window& parent) {}

		INFLUX_PLATFORM_API
		virtual void set_owner(window& owner) {}

		INFLUX_PLATFORM_API
		virtual rect adjust_rect(const rect& rect) { return rect; }

		INFLUX_PLATFORM_API
		virtual ~window() = default;

		void INFLUX_PLATFORM_API request_quit();

		bool INFLUX_PLATFORM_API has_quit_request() const;

		inline float get_aspect_ratio(e_space space = e_space::client) const
		{
			return get_rect(space).get_aspect_ratio();
		}

	protected:
		window(const window_desc& desc);
		window_desc m_desc{};
		bool m_has_quit_event = false;
	};
}