#pragma once
#include "influx_graphics/common.h"
#include "core/string.h"

namespace influx::graphics
{
	// interface base class for each object created by our graphics api
	class base
	{
		virtual void on_set_name(const debug_name& name) {}

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
		}

		inline const debug_name& get_name() const
		{
			return m_name;
		}

		base(const base&) = delete;
		base(base&&) = delete;
		base& operator=(const base&) = delete;
		base& operator=(base&&) = delete;
		virtual ~base() = default;

	protected:
		base() = default;
		void* mp_native;
		
	private:
		debug_name m_name{};
	};
}