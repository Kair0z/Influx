#pragma once

// influx::core
#include "basetypes.h"
#include "core/string.h"
#include "core/container/map.h"
#include "core/debug.h"

namespace influx
{
	enum class e_result : uint8
	{
		success,
		warning,
		error,
		count
	};

	namespace detail
	{
		constexpr const char* error_codes[]
		{
			"success",
			"warning",
			"error"
		};

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

	template <typename _t = char, typename _e = const char*>
	class result final
	{
		using ex_type = _t;
		using unex_type = _e;

		unex_type m_unexpected = {};
		ex_type m_expected = {};

	public:
		// constructors
		result() : m_expected{}, m_unexpected{} {}
		result(const _t& value) { m_expected = value; }
		result(const _e& error) { m_unexpected = error; }

		inline static result make_error(const _e& error)
		{
			return result(error);
		}

		// if underlying _t is 'boolable', this will only return true if
		//		this is success (expected)
		//		AND this value == true
		// else, this returns only this is success
		operator bool() const
		{
			// Compile-time branch if T has operator bool
			if constexpr (detail::has_bool_operator_v<_t> || detail::is_booleable_v<_t>) 
			{
				return is_success() && m_expected;
			}
			
			return is_success();
		}

		bool operator!() const
		{
			return !*this;
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

		bool is_success() const
		{
			return m_unexpected == _e{};
		}

		bool is_unex() const
		{
			return !is_success();
		}

		const _t& get_safe() const
		{
			return m_expected;
		}

		_t& get()
		{
#if INFLUX_DEBUG
			influx_assert(is_success());
#endif
			return m_expected;
		}

		const _t& get() const
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
	};
}