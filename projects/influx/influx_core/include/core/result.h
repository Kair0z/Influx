#pragma once

// influx::core
#include "basetypes.h"

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
	}

	class result final
	{
	public:
		result() = default;

		result(bool value)
		{
			m_value = value ? (uint64)e_result::success : (uint64)e_result::error;
		}

		result(e_result result)
		{
			m_value = (uint64)result;
		}

		result(uint64 value)
		{
			m_value = value;
		}

		result(void* ptr)
		{
			m_value = (ptr == nullptr) ? (uint64)e_result::error : (uint64)e_result::success;
		}

		operator bool() const
		{
			return is_success();
		}

		bool is_success() const
		{
			return m_value == (uint64)e_result::success;
		}

		const char* print() const
		{
			return detail::error_codes[m_value];
		}

		void set(e_result result)
		{
			m_value = (uint64)result;
		}

		e_result get() const
		{
			return (e_result)m_value;
		}

	private:
		uint64 m_value = 0u;
	};
}