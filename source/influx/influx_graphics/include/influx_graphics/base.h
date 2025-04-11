#pragma once

// influx::graphics
#include "influx_graphics/common.h"

// influx::core
#include "core/string.h"

namespace influx::graphics
{
	class device;

	/* rhi object base interface */
	class base
	{
		virtual void set_name_impl(const debug_name& name) {}
		virtual void release_impl(device*) = 0;

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
			return !is_released() && mp_native != nullptr;
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

		virtual void release(device* device)
		{
			m_is_released = true;
			release_impl(device);
		}

		bool is_released() const
		{
			return m_is_released;
		}

	protected:
		// only the child classes can call constructor/desctructor
		base() = default;
		virtual ~base();

		void* mp_native = nullptr;

		base(const base&) = delete;
		base(base&&) = delete;
		base& operator=(const base&) = delete;
		base& operator=(base&&) = delete;
		
	private:
		debug_name m_name{};
		bool m_is_released = false;

		friend class device;
	};
}