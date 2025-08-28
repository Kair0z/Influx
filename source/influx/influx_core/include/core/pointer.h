#pragma once

#ifndef __CORE_POINTER_H_
#define __CORE_POINTER_H_

#include <memory>

namespace influx
{
	namespace pointers
	{
		template <typename _t>
		class weak_ptr;

		template <typename _t>
		class shared_ptr final
		{
			// a shared pointer keeps a control block,
			// which it passes to copies.
			// each copy increments the refcount
			// when the last reference destroys, we free the memory
			struct ctl_block final
			{
				uint64 m_refcount = 0u;
				uint64 m_weakcount = 0u;
				void (*m_delete_fnc)(_t*);
				_t* m_ptr = nullptr;
				
				ctl_block(_t* ptr, void(*d)(_t*))
				{
					m_refcount = 1u;
					m_weakcount = 0u;
					m_delete_fnc = d;
					m_ptr = ptr;
				}
			};
			ctl_block* m_ctl = nullptr;
			void release()
			{
				if (m_ctl == nullptr) return;
				if (--m_ctl->m_refcount == 0u)
				{
					m_ctl->m_delete_fnc(m_ctl->m_ptr);
					delete m_ctl;
				}
				m_ctl = nullptr;
			}

			// a weak_ptr exclusively has access to our ctl_block
			friend class weak_ptr<_t>;

		public:
			// constructors
			shared_ptr() = default;
			explicit shared_ptr(_t* ptr, void(*delete_fnc)(_t*))
			{
				m_ctl = (ptr != nullptr) ? new ctl_block(ptr, delete_fnc) : nullptr;
			}
			explicit shared_ptr(_t* ptr) : shared_ptr(ptr, [](_t* p) { delete p; }) { }
			shared_ptr& operator=(const shared_ptr& other)
			{
				if (this != &other) 
				{
					release();
					m_ctl = other.m_ctl;
					if (m_ctl) ++m_ctl->m_refcount;
				}
				return *this;
			}
			shared_ptr& operator=(shared_ptr&& other)
			{
				if (this != &other)
				{
					release();
					m_ctl = other.m_ctl;
					other.m_ctl = nullptr;
				}
				return *this;
			}
			shared_ptr(const shared_ptr& other)
			{
				*this = other;
			}
			shared_ptr(shared_ptr&& other)
			{
				*this = other;
			}
			~shared_ptr() { release(); }

			// interface
			void reset(_t* new_ptr, void(*delete_fnc)(_t*) = [](_t* p) { delete p; })
			{
				release();
				if (new_ptr)
				{
					*this = shared_ptr(new_ptr, delete_fnc);
				}
			}

			const _t* get() const
			{
				return m_ctl ? m_ctl->m_ptr : nullptr;
			}
			uint64 get_refcount() const
			{
				return m_ctl ? m_ctl->m_refcount : 0u;
			}
			explicit operator bool() const
			{
				return get() != nullptr;
			}

			const _t& operator*() const
			{
				return *m_ctl->m_ptr;
			}
			const _t* operator->() const
			{
				return get();
			}
		};
	
		template <typename _t, class ..._args>
		shared_ptr<_t> make_shared(_args&&... args)
		{
			return shared_ptr<_t>(new _t(std::forward<_args>(args)...));
		}
		
		template <typename _t>
		class weak_ptr final
		{
			using shared_ptr = shared_ptr<_t>;

			using ctl_block = shared_ptr::ctl_block;
			ctl_block* m_ctl = nullptr;

		public:
			void reset()
			{
				if (m_ctl == nullptr) return;
				if (--m_ctl->m_weakcount == 0u && m_ctl->m_refcount == 0u)
				{
					delete m_ctl;
				}
				m_ctl = nullptr;
			}

			weak_ptr() = default;
			weak_ptr(const shared_ptr& shared)
			{
				m_ctl = shared.m_ctl;
				if (m_ctl)
				{
					++m_ctl->m_weakcount;
				}
			}
			weak_ptr(const weak_ptr& weak)
			{
				m_ctl = weak.m_ctl;
				if (m_ctl)
				{
					++m_ctl->m_weakcount;
				}
			}
			weak_ptr& operator=(const weak_ptr& other)
			{
				if (this != &other)
				{
					reset();
					m_ctl = other.m_ctl;
					if (m_ctl) ++m_ctl->m_weakcount;
				}
				return *this;
			}
			weak_ptr& operator=(weak_ptr&& other)
			{
				if (this != &other)
				{
					reset();
					m_ctl = other.m_ctl;
					other.m_ctl = nullptr;
				}
				return *this;
			}
			~weak_ptr()
			{
				reset();
			}
			
			// has the object been released yet?
			bool is_expired() const
			{
				return m_ctl == nullptr || m_ctl->m_refcount == 0u;
			}

			// make a shared pointer of our pointer,
			// so we 'lock' it from being released
			shared_ptr lock() const
			{
				if (is_expired()) 
					return shared_ptr();
			
				++m_ctl->m_refcount;
				return shared_ptr(m_ctl);
			}

			uint64 get_refcount() const
			{
				return m_ctl ? m_ctl->m_refcount : 0u;
			}
			uint64 get_weakcount() const
			{
				return m_ctl ? m_ctl->m_weakcount : 0u;
			}
		};
		
		template <typename _t, class _delfunc = void(*)(_t*)>
		class unique_ptr final
		{
			_t* m_ptr = nullptr;
			_delfunc m_deleter = nullptr;

		public:
			void reset(_t* new_ptr = nullptr)
			{
				if (m_ptr) m_deleter(m_ptr);
				m_ptr = new_ptr;
			}

			// doesn't free the memory, 
			// rather, we retrieve our pointer out of the confines of the unique_ptr
			_t* release()
			{
				_t* tmp = m_ptr; 
				m_ptr = nullptr;
				return tmp;
			}

			void swap(unique_ptr& other) noexcept
			{
				std::swap(m_ptr, other.m_ptr);
				std::swap(m_deleter, other.m_deleter);
			}

			// constructors
			explicit unique_ptr(_t* p = nullptr, _delfunc d = [](_t* p) { delete p; })
			{
				m_ptr = p;
				m_deleter = d;
			}
			unique_ptr& operator=(const unique_ptr&) = delete;
			unique_ptr& operator=(unique_ptr&& other) noexcept
			{
				m_ptr = other.m_ptr;
				m_deleter = other.m_deleter;
				other.m_ptr = nullptr;
			}
			unique_ptr(const unique_ptr&) = delete;
			unique_ptr(unique_ptr&& other)
			{
				*this = other;
			}
			~unique_ptr()
			{
				reset();
			}
			
			// observers
			const _t* get() const 
			{ 
				return m_ptr;
			}
			explicit operator bool() const 
			{ 
				return m_ptr != nullptr; 
			}
			const _t& operator*() const 
			{ 
				return *m_ptr;
			}
			const _t* operator->() const 
			{ 
				return get();
			}
		};

		template <class _t, class... _args>
		unique_ptr<_t> make_unique(_args&&... args) 
		{
			return unique_ptr<_t>(new _t(std::forward<_args>(args)...));
		}
	}
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

#if 0
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
#endif
}

#endif