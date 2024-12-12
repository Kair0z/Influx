#pragma once

#ifndef __CORE_POINTER_H_
#define __CORE_POINTER_H_

#include <memory>

namespace influx
{
	template <typename _t>
	using ptr = _t*;

	// pointer that points to an immutable object
	template <typename _t>
	using cptr = _t const*;

	template <typename _t>
	constexpr bool is_null(const ptr<_t> p)
	{
		return (p == nullptr);
	}

	template <typename _t>
	using shared_ptr = std::shared_ptr<_t>;

	template <typename _t>
	using uni_ptr = std::unique_ptr<_t>;

	template <typename _t>
	using weak_ptr = std::weak_ptr<_t>;

	// lightweight ref counting pointer
	// it DOES NOT de-allocate the pointed object
	template <typename _t>
	class ref_ptr final
	{
		using my_type = _t;
		my_type* m_ptr;
		uint32* m_refcount;

	public:
		ref_ptr(_t* ptr = nullptr)
			: m_ptr{ ptr }, m_refcount(ptr ? new uint32(1u) : nullptr) {}

		// copy
		ref_ptr(const ref_ptr<_t>& other)
			: m_ptr { other.m_ptr }
			, m_refcount{ other.m_refcount }
		{
			if (m_refcount) ++(*m_refcount);
		}

		// move
		ref_ptr(ref_ptr<_t>&& other) noexcept
			: m_ptr(other.m_ptr)
			, m_refcount(other.m_refcount)
		{
			other.m_ptr = nullptr;
			other.m_refcount = nullptr;
		}
		
		// destructor
		~ref_ptr() 
		{
			release();
		}

		// copy
		ref_ptr<_t>& operator=(const ref_ptr<_t>& other)
		{
			if (this != &other) 
			{
				release();
				m_ptr = other.m_ptr;
				m_refcount = other.m_refcount;
				if (m_refcount)
				{
					++(*m_refcount);
				}
			}
			return *this;
		}

		// move
		ref_ptr<_t>& operator=(ref_ptr<_t>&& other) noexcept 
		{
			if (this != &other) 
			{
				release();
				m_ptr = other.m_ptr;
				m_refcount = other.m_refcount;
				other.m_ptr = nullptr;
				other.m_refcount = nullptr;
			}
			return *this;
		}

		_t& operator*() const { return *m_ptr; }
		_t* operator->() const { return get_ptr(); }

		explicit operator bool() const 
		{ 
			return m_ptr != nullptr; 
		}

		uint32 get_refcount() const 
		{ 
			return m_refcount ? *m_refcount : 0;
		}

		void release()
		{
			if (m_refcount)
			{
				--(*m_refcount);
			}
		}

		_t* get_ptr() const { return m_ptr; }
		_t const* get_ptr() { return m_ptr; }
	};
}

#endif