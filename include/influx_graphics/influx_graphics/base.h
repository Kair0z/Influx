#pragma once
#include "influx_graphics/common.h"
#include "core/string.h"

namespace influx::graphics
{
	class base
	{
		virtual void set_name_impl(const debug_name& name) {}

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

		inline void set_name(const debug_name& name)
		{
			m_name = name;
			set_name_impl(name);
		}

		inline const debug_name& get_name() const
		{
			return m_name;
		}

		base(const base&) = delete;
		base(base&&) = delete;
		base& operator=(const base&) = delete;
		base& operator=(base&&) = delete;

		virtual ~base();

		virtual void release() = 0;

	protected:
		base() = default;
		void* mp_native = nullptr;
		
	private:
		debug_name m_name{};
	};
}