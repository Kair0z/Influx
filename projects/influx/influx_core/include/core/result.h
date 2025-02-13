#pragma once

// influx::core
#include "basetypes.h"
#include "core/string.h"
#include "core/container/map.h"

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

	template <typename _t, typename _e = e_result>
	class result final
	{
		using ex_type = _t;
		using unex_type = _e;

		ex_type m_expected = {};
		unex_type m_unexpected = {};

	public:
		// constructors
		result() : m_expected{}, m_unexpected{} {}
		result(const _t& value) { m_expected = value; }
		result(const _e& error) { m_unexpected = error; }

		// if underlying _t is 'boolable' that plays a part in the evaluation of this result
		operator bool() const
		{
			// Compile-time branch if T has operator bool
			if constexpr (detail::has_bool_operator_v<_t> || detail::is_booleable_v<_t>) 
			{
				return is_success() && m_expected;
			}
			
			return is_success();
		}

		bool is_success() const
		{
			return m_unexpected == _e{};
		}

		const _t& get() const
		{
			return m_expected;
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