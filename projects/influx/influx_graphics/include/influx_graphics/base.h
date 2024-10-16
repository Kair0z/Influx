#pragma once
#include "influx_graphics/common.h"
#include <string>

namespace influx::graphics
{
	// interface base class for each object created by our graphics api
	class base
	{
	public:
		template <typename _t>
		inline _t* get_native() const
		{
			return (_t*)(mp_native);
		}

		template <typename _t>
		inline _t*& get_native()
		{
			return (_t*&)(mp_native);
		}

		inline bool is_valid() const
		{
			return mp_native != nullptr;
		}

		inline operator bool() const
		{
			return is_valid();
		}

#if _DEBUG
		inline void set_name(const string& new_name)
		{
			m_debug_name = new_name;

			set_name_impl(new_name);
		}

		inline const string& get_name() const
		{
			return m_debug_name;
		}
#endif

	protected:
		base() = default;
		virtual ~base() = default;

		void* mp_native;
#if _DEBUG
		string m_debug_name;
		virtual void set_name_impl(const string& new_name) { }
#endif
	};
}