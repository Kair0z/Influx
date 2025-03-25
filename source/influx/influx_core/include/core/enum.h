#pragma once

#include "debug.h"
#include <type_traits>
#include <concepts>
#include <bit>

namespace influx
{
	template <typename _e> requires std::is_enum_v<_e>
	struct enum_bitmask_operators
	{
		static constexpr inline bool enable = false;
	};
	template <typename _e>
	inline constexpr bool enable_bitmask_operators = enum_bitmask_operators<_e>::enable;

	template <typename E>
	inline typename std::enable_if_t<enable_bitmask_operators<E>, E> operator|(E lhs, E rhs)
	{
		using T = std::underlying_type_t<E>;
		return static_cast<E>(static_cast<T>(lhs) | static_cast<T>(rhs));
	}
	template <typename E>
	inline typename std::enable_if_t<enable_bitmask_operators<E>, E>::type operator&(E lhs, E rhs)
	{
		using T = std::underlying_type_t<E>;
		return static_cast<E>(static_cast<T>(lhs) & static_cast<T>(rhs));
	}
	template <typename E>
	inline typename std::enable_if_t<enable_bitmask_operators<E>, E> operator^(E lhs, E rhs)
	{
		using T = std::underlying_type_t<E>;
		return static_cast<E>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
	}
	template <typename E>
	inline typename std::enable_if_t<enable_bitmask_operators<E>, E> operator~(E e)
	{
		using T = std::underlying_type_t<E>;
		return static_cast<E>(~static_cast<T>(e));
	}
	template <typename E>
	inline typename std::enable_if_t<enable_bitmask_operators<E>, E&> operator|=(E& lhs, E rhs)
	{
		using T = std::underlying_type_t<E>;
		return lhs = static_cast<E>(static_cast<T>(lhs) | static_cast<T>(rhs));
	}
	template <typename E>
	inline typename std::enable_if_t<enable_bitmask_operators<E>, E&> operator&=(E& lhs, E rhs)
	{
		using T = std::underlying_type_t<E>;
		return lhs = static_cast<E>(static_cast<T>(lhs) & static_cast<T>(rhs));
	}
	template <typename E>
	inline typename std::enable_if_t<enable_bitmask_operators<E>, E&> operator^=(E& lhs, E rhs)
	{
		using T = std::underlying_type_t<E>;
		return lhs = static_cast<E>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
	}

	template<typename _enum> requires std::is_enum_v<_enum>
	inline constexpr bool has_all_flags(_enum value, _enum flags)
	{
		using T = std::underlying_type_t<_enum>;
		return (((T)value) & (T)flags) == ((T)flags);
	}
	template<typename _enum> requires std::is_enum_v<_enum>
	inline constexpr bool has_any_flag(_enum value, _enum flags)
	{
		using T = std::underlying_type_t<_enum>;
		return (((T)value) & (T)flags) != 0;
	}
	template<typename _enum> requires std::is_enum_v<_enum>
	inline constexpr bool has_flag(_enum value, _enum flag)
	{
		using T = std::underlying_type_t<_enum>;
		influx_assert(std::has_single_bit((T)flag));
		return has_any_flag(value, flag);
	}
	template<typename _enum> requires std::is_enum_v<_enum>
	inline _enum& set_flag(_enum& value, _enum flag, bool enabled)
	{
		if (enabled)
		{
			value |= flag;
		}
		else
		{
			value &= ~flag;
		}
		return value;
	}
}

#define ENABLE_ENUM_BIT_OPERATORS(type) \
	template<> struct influx::enum_bitmask_operators<type> { static constexpr inline bool enable = true; }