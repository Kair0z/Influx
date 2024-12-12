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

	template <typename _t = bool>
	class result final
	{
	public:
		using underlying = _t;

		// default constructor 
		result() : m_value{}, m_return{true} {}

		// takes the underlying value, default means success
		result(const _t& value) : m_return{ value } { set_state(e_result::success);  }
		result(_t&& value) : m_return{ value } { set_state(e_result::success); }

		template <typename ..._args>
		result(_args&&... args) : m_return{ args... }
		{
			set_state(e_result::success);
		}

		result(e_result result) { m_value = (uint64)result; }
		result(uint64 value) { m_value = value; }
		result(void* ptr) { m_value = (ptr == nullptr) ? (uint64)e_result::error : (uint64)e_result::success; }

		static result make_error()
		{
			result new_result = result(e_result::error);
			return new_result;
		}

		static result make_warning()
		{
			result new_result = result(e_result::warning);
			return new_result;
		}

		// if underlying _t is 'boolable' that plays a part in the evaluation of this result
		operator bool() const
		{
			// Compile-time branch if T has operator bool
			if constexpr (detail::has_bool_operator_v<_t> || detail::is_booleable_v<_t>) 
			{
				return is_success() && m_return;
			}
			
			return is_success();
		}

		bool is_success() const
		{
			return m_value == (uint64)e_result::success;
		}

		const char* to_string() const
		{
			return detail::error_codes[m_value];
		}

		void set_error()
		{
			set_state(e_result::error);
		}

		void set_warning()
		{
			set_state(e_result::warning);
		}

		bool is_error() const
		{
			return get_state() == e_result::error;
		}

		bool is_warning() const
		{
			return get_state() == e_result::warning;
		}

		void set_state(e_result result)
		{
			m_value = (uint64)result;
		}

		e_result get_state() const
		{
			return (e_result)m_value;
		}

		const _t& get() const
		{
			return m_return;
		}

		void set(const _t& value)
		{
			m_return = value;
		}

		// appending results to eachother
		template <typename _tresult>
		const _tresult& operator+=(const _tresult& other)
		{
			return append<_tresult>(other);
		}

		template <typename _tresult>
		const _tresult& append(const _tresult& other)
		{
			if (other.is_error())
			{
				(*this).set_error();
			}
			else if (other.is_warning() && !this->is_error())
			{
				(*this).set_warning();
			}

			return other;
		}

		template <typename _tresult, typename _t = _tresult::underlying>
		const _t& append_and_get(const _tresult& other)
		{
			return append<_tresult>(other).get();
		}

		_t& operator->()
		{
			return m_return;
		}

		const _t& operator->() const
		{
			return m_return;
		}

	private:
		uint64 m_value = 0u;
		_t m_return{};
	};
}