#pragma once

// influx::core
#include "basetypes.h"
#include "core/string.h"
#include "core/container/map.h"
#include "core/debug.h"

namespace influx
{
	namespace detail
	{
		// Helper trait to detect operator bool
		template <typename T, typename = void>
		struct has_bool_operator : std::false_type {};

		// Specialization for types that have operator bool
		template <typename T>
		struct has_bool_operator<T, std::void_t<decltype(std::declval<T>().operator bool())>> : std::true_type {};

		// Helper variable template
		template <typename T>
		inline constexpr bool has_bool_operator_v = has_bool_operator<T>::value;

		// Trait to check if a type is booleable
		template <typename T>
		struct is_booleable : std::is_convertible<T, bool> {};

		// Helper variable template
		template <typename T>
		inline constexpr bool is_booleable_v = is_booleable<T>::value;
	}

	// enable this to sanitize your program.
	// by default, only in DEBUG when .get() is called on !result.is_success()
	// will we assert the result is valid.
	// with this enabled, we assert as soon as any error is made (exactly where it occured)
#define CHECK_RESULT_IMMEDIATE 1

	template <typename _t = unsigned char, typename _e = const char*>
	class result final
	{
		using ex_type = _t;
		using unex_type = _e;

		ex_type m_expected = {};
		unex_type m_unexpected = {};

		// warnings carry a valid result,
		// AND an unexpected result,
		// but return true on success
		bool m_is_warning = false;

	public:
		// constructors
		result() : m_expected{}, m_unexpected{} {}
		result(_t&& value) : m_expected{ value }, m_unexpected{} {}
		result(const _t& value) { m_expected = value; }
		result(const _e& error) { m_unexpected = error; }

		// forbid implicit conversion!
		template <typename = std::enable_if_t<!std::is_same_v<_t, bool>>>
		operator _t() const = delete;

		inline static result make_success(const _t& expected = {})
		{
			return result( expected );
		}

		inline static result make_error(const _e& error)
		{
#if CHECK_RESULT_IMMEDIATE && INFLUX_DEBUG
			result res = result(error);
			influx_assert_msg(res.is_success(), res.m_unexpected);
			return res;
#else
			return result(error);
#endif
		}

		inline static result make_warning(const _t& value, const _e& warning)
		{
			result res{};
			res.m_expected = value;
			res.m_unexpected = warning;
			res.m_is_warning = true;
			return res;
		}

		template <typename _ot>
		inline static result make_error(const result<_ot>& other_error )
		{
#if CHECK_RESULT_IMMEDIATE
			result res = result(other_error.get_unex());
			res.get();
			return res;
#else
			return result( other_error.get_unex() );
#endif
		}

		// if underlying _t is 'boolable', this will only return true if
		//		this is success (expected)
		//		AND this value == true
		// else, this returns only this is success
		operator bool() const
		{
			// Compile-time branch if T has operator bool
#if 0
			if constexpr (detail::has_bool_operator_v<_t> || detail::is_booleable_v<_t>) 
			{
				return is_success() && m_expected;
			}
#endif		
			return is_success();
		}

		bool operator!() const
		{
			return !((*this).operator bool());
		}

		bool operator==(unex_type result)
		{
			return result == m_unexpected;
		}

		bool operator!=(unex_type result)
		{
			return result != m_unexpected;
		}

		bool operator==(ex_type type)
		{
			return type == m_expected;
		}

		bool operator!=(ex_type type)
		{
			return type != m_expected;
		}

		// true if unexpected not set || is_warning
		inline bool is_success() const
		{
			return !is_fail();
		}

		// true if unexpected value is set (can be warning!)
		inline bool is_unex() const
		{
			return m_unexpected != _e{};
		}

		// true if is_fail() AND !warning
		inline bool is_fail() const
		{
			return is_unex() && m_is_warning == false;
		}

		const _t& get_safe() const
		{
			return m_expected;
		}

		_t& get_safe()
		{
			return m_expected;
		}

		inline _t& get()
		{
#if INFLUX_DEBUG
			influx_assert_msg(is_success(), m_unexpected);
			if (m_is_warning)
				std::wcout << L"warning!: " << m_unexpected << L"\n";
#endif
			return m_expected;
		}

		inline const _t& get() const
		{
			return m_expected;
		}

		const unex_type& get_unex() const
		{
			return m_unexpected;
		}

		unex_type& get_unex()
		{
			return m_unexpected;
		}

		void set(const _t& value)
		{
			m_expected = value;
		}

		_t& operator->()
		{
			return m_expected;
		}

		const _t& operator->() const
		{
			return m_expected;
		}

		friend std::ostream& operator<<(std::ostream& os, const result& res)
		{
			if (res.is_unex())
			{
				os << res.get_unex() << " | ";
			}
			os << res.get();
		}
	};
}