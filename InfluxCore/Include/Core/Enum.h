#pragma once

// Based on:
// https://gist.github.com/HertzDevil/43e8acbe5d73aba7610dcc1e79f17f3d

#ifndef _CORE_ENUM_H_
#define _CORE_ENUM_H_

#define __CORE_ENUM_USECORE_ 0
#define __CORE_ENUM_NAME_MAX	Max
#define __CORE_ENUM_NAME_MIN	Min
#define __CORE_ENUM_NAME_NONE	Default

#include <type_traits>
#include <limits>

// Dictionary:
// _E:		The enum value-type
// _C:		The enum category-type
// _V:		The enum listed values

// E-min:	Minimum given value of the enum
// E-max:	Maximum given value of the enum
// E-none:	None assigned value of the enum

#if __CORE_ENUM_USECORE_
#include "Core/Math/Math.h"
#else
#include <algorithm>
namespace Influx::Math
{
	template <typename _T>
	constexpr _T Clamp(const _T& x, const _T& lo, const _T& hi) noexcept
	{
		return x < lo ? lo : (x > hi ? hi : x);
	}

	template <typename _T,
		typename = std::enable_if_t<std::is_unsigned_v<std::decay_t<_T>>>>
		constexpr _T ClampBitwise(_T x, _T lo, _T hi) noexcept
	{
		return (x & hi) | lo;
	}

	namespace Internal
	{
		template <typename _T>
		constexpr _T const& DoMax(_T const& v) { return v; }

		template <typename _T, class... _R>
		constexpr _T const& DoMax(_T const& v0, _T const& v1, _R const&... rest)
		{
			return DoMax(v0 < v1 ? v1 : v0, rest...);
		}

		template <typename _T>
		constexpr _T const& DoMin(_T const& v) { return v; }

		template <typename _T, class... _R>
		constexpr _T const& DoMin(_T const& v0, _T const& v1, _R const&... rest)
		{
			return DoMin(v0 < v1 ? v0 : v1, rest...);
		}
	}

	template <typename _T, class... _R>
	inline constexpr _T const& Max(_T const& a, _R const&... rest)
	{
		return Internal::DoMax(a, rest...);
	}

	template <typename _T, class... _R>
	inline constexpr _T const& Min(_T const& a, _R const&... rest)
	{
		return Internal::DoMin(a, rest...);
	}
}


#endif

namespace Influx::Enum
{
	// Default C++ Enum
	// Conversion == static_cast
	struct EnumDefault {};

	// [Standard Safe Enum]
	// Out of range:	'none-element'
	// Requires:		E-min, E-max, E-none
	struct EnumStandard {};

	struct EnumLinear {};

	struct EnumBitmask {};

	template <typename _E, _E... _V>
	struct EnumDiscrete {};

	template <typename _E>
	struct EnumTraits { using Category = EnumDefault; };

	template <typename _C>
	struct EnumCategoryTraits;

	// [Categories]
	namespace Internal
	{
		template <typename _E>
		constexpr bool isScopedEnum_f(std::false_type) noexcept { return false; }

		template <typename _E>
		constexpr bool isScopedEnum_f(std::true_type) noexcept { return !std::is_convertible_v<_E, std::underlying_type_t<_E>>; }

		template <typename _E>
		struct isScopedEnum : std::integral_constant<bool, isScopedEnum_f<_E>(std::is_enum<_E>()) {};

		template <typename _E>
		inline constexpr bool isScopedEnum_v = isScopedEnum<_E>::value;

		template <typename _E>
		constexpr std::underlying_type<_E> value_cast_impl(_E x) noexcept { return static_cast<std::underlying_type_t<_E>>(x); }

		template <typename _C>
		struct IsEnumCategoryDiscrete : std::false_type {};

		template <typename _E, _E... _V>
		struct IsEnumCategoryDiscrete<Enum::EnumDiscrete<_E, _V...>> : std::true_type {};

		template <typename _C, typename = void>
		struct IsEnumCategory : std::false_type {};

		template <typename _C>
		struct IsEnumCategory<_C, std::void_t<typename Enum::EnumCategoryTraits<_C>::Category>> : std::true_type {};

		template <typename _E, typename = void>
		struct GetEnumCategory { using type = EnumDefault; };

		template <typename _E>
		struct GetEnumCategory<_E, std::void_t<typename Enum::EnumTraits<_E>::Category>>
		{
		private:
			using T2 = typename Enum::EnumTraits<_E>::Category;
		public:
			using type = std::conditional_t<IsEnumCategory<T2>::value, T2, Enum::EnumDefault>;
		};
	}

	template <typename _C>
	using IsEnumCategory = Internal::IsEnumCategory<_C>;

	template <typename _C>
	inline constexpr bool bIsEnumCategory = IsEnumCategory<_C>::value;

	template <typename _E>
	using GetEnumCategory = Internal::GetEnumCategory<_E>;

	template <typename _E>
	using GetEnumCategoryT = typename GetEnumCategory<_E>::type; // If no valid category is found, defaults to DefaultEnum
	

	// [Members]
#pragma region Members
	namespace Internal
	{
		template <typename _E, typename = void>
		struct EnumHasNoneMember : std::false_type {};
		template <typename _E>
		struct EnumHasNoneMember<_E, std::void_t<decltype(_E::__CORE_ENUM_NAME_NONE)>> : std::true_type {};

		template <typename _E, typename = void>
		struct EnumHasMinMember : std::false_type { };
		template <typename _E>
		struct EnumHasMinMember<_E, std::void_t<decltype(_E::__CORE_ENUM_NAME_MIN)>> : std::true_type {};

		template <typename _E, typename = void>
		struct EnumHasMaxMember : std::false_type { };
		template <typename _E>
		struct EnumHasMaxMember<_E, std::void_t<decltype(_E::__CORE_ENUM_NAME_MAX)>> : std::true_type {};
	}

	template <typename _E, typename = std::enable_if_t<std::is_enum_v<_E>>>
	constexpr bool HasNone() noexcept { return Internal::EnumHasNoneMember<_E>::value; }

	template <typename _E, typename = std::enable_if_t<std::is_enum_v<_E>>>
	constexpr _E GetNone() noexcept { if constexpr (Internal::EnumHasNoneMember<_E>::value) return _E::__CORE_ENUM_NAME_NONE; }

	// Checks whether the given enumeration type has a minimum element.
	template <typename _E, typename = std::enable_if_t<std::is_enum_v<_E>>>
	constexpr bool HasMin() noexcept { return !Internal::IsEnumCategoryDiscrete<GetEnumCategoryT<_E>>::value; }

	// Obtains the minimum element of a given enumeration type.
	template <typename _E, typename = std::enable_if_t<std::is_enum_v<_E>&& HasMin<_E>()>>
	constexpr _E GetMin() noexcept 
	{
		if constexpr (Internal::EnumHasMinMember<_E>::value) return _E::__CORE_ENUM_NAME_MIN;
		if constexpr (!Internal::IsEnumCategoryDiscrete<GetEnumCategoryT<_E>>::value) 
			return _E{ std::numeric_limits<std::underlying_type_t<_E>>::__CORE_ENUM_NAME_MIN };
	}

	// Checks whether the given enumeration type has a maximum element.
	template <typename _E, typename = std::enable_if_t<std::is_enum_v<_E>>>
	constexpr bool HasMax() noexcept { return !Internal::IsEnumCategoryDiscrete<GetEnumCategoryT<_E>>::value; }

	// Obtains the maximum element of a given enumeration type.
	template <typename _E, typename = std::enable_if_t<std::is_enum_v<_E>&& HasMax<_E>()>>
	constexpr _E GetMax() noexcept
	{
		if constexpr (Internal::EnumHasMaxMember<_E>::value) return _E::__CORE_ENUM_NAME_MAX;
		if constexpr (!Internal::IsEnumCategoryDiscrete<GetEnumCategoryT<_E>>::value)
			return _E{ std::numeric_limits<std::underlying_type_t<_E>>::__CORE_ENUM_NAME_MAX };
	}
#pragma endregion

	// [Casting]
#pragma region Casting
	// Casts an enumeration value to its underlying type.
	template <typename _E, typename _CTraits = EnumCategoryTraits<GetEnumCategoryT<_E>>, typename std::enable_if_t<std::is_enum_v<_E>>>
	constexpr std::underlying_type_t<_E> ValueCast(_E x) noexcept
	{
		static_assert(_CTraits::template Valid<_E>());
		return Internal::value_cast_impl(x);
	}

	// Casts a value to the given enumeration type.
	template <typename _E, typename _VT, typename _CTraits = EnumCategoryTraits<GetEnumCategoryT<_E>>,
		typename std::enable_if_t<std::is_enum_v<_E>>,
		typename std::enable_if_t<std::is_convertible_v<_VT, std::underlying_type_t<_E>>>>
	constexpr _E EnumCast(_VT x) noexcept
	{
		static_assert(_CTraits::template Valid<_E>());
		return _CTraits::template EnumCast<_E>(x);
	}

	// Constrains a given value by the given enumeration type, as if by casting it
	// to and from the enumeration type.
	template <typename _E, typename _VT,
		typename = std::enable_if_t<std::is_enum_v<_E>>,
		typename = std::enable_if_t<std::is_same_v<_VT, std::underlying_type_t<_E>>>>
	constexpr std::underlying_type_t<_E> ValueCast(_VT x) noexcept 
	{
		return ValueCast(EnumCast<_E>(x));
	}

	// Constrains the given enumeration type, as if by casting it to and from its
	// underlying type.
	template <typename _E, typename _VT,
		typename = std::enable_if_t<std::is_enum_v<_E>>,
		typename = std::enable_if_t<std::is_same_v<_VT, std::underlying_type_t<_E>>>>
		constexpr std::underlying_type_t<_E> EnumCast(_VT x) noexcept
	{
		return EnumCast<_E>(ValueCast(x));
	}
#pragma endregion

	// [Operators]
#pragma region Operators
	template <typename _E,
		typename = std::enable_if_t<Internal::isScopedEnum_v<_E>>,
		typename = std::enable_if_t<HasNone<_E>()>>
	constexpr bool operator!(const _E& lhs) noexcept
	{
		return lhs == GetNone<_E>();
	}

	template <typename _E,
		typename = std::enable_if_t<Internal::isScopedEnum_v<_E>>,
		typename = std::enable_if_t<std::is_same_v<GetEnumCategoryT<_E>, EnumLinear>>>
	constexpr _E& operator++(_E& lhs) noexcept
	{
		if (!(!lhs || lhs == GetMax<_E>())) 
			lhs = EnumCast<_E>(ValueCast(lhs) + 1);

		return lhs;
	}

	template <typename _E,
		typename = std::enable_if_t<Internal::isScopedEnum_v<_E>>,
		typename = std::enable_if_t<std::is_same_v<GetEnumCategoryT<_E>, EnumLinear>>>
	constexpr _E operator++(const _E& lhs) noexcept
	{
		_E ret = lhs;
		++lhs;
		return ret;
	}

	template <typename _E,
		typename = std::enable_if_t<Internal::isScopedEnum_v<_E>>,
		typename = std::enable_if_t<std::is_same_v<GetEnumCategoryT<_E>, EnumLinear>>>
		constexpr _E& operator--(_E& lhs) noexcept
	{
		if (!(!lhs || lhs == GetMin<_E>()))
			lhs = EnumCast<_E>(ValueCast(lhs) - 1);

		return lhs;
	}

	template <typename _E,
		typename = std::enable_if_t<Internal::isScopedEnum_v<_E>>,
		typename = std::enable_if_t<std::is_same_v<GetEnumCategoryT<_E>, EnumLinear>>>
		constexpr _E operator--(const _E& lhs) noexcept
	{
		_E ret = lhs;
		--lhs;
		return ret;
	}

	// If neither operand is _E::None, returns the union of the two bit masks.
	// Otherwise returns _E::None. Only supports bitmask enum class types.
	template <typename _E,
		typename = std::enable_if_t<Internal::isScopedEnum_v<_E>>,
		typename = std::enable_if_t<std::is_same_v<GetEnumCategoryT<_E>, EnumBitmask>>>
	constexpr _E operator|(const _E& lhs, const _E& rhs) noexcept 
	{
		if constexpr (HasNone<_E>())
			if (!lhs || !rhs)
				return GetNone<_E>();

		return EnumCast<_E>(static_cast<std::underlying_type_t<_E>>(ValueCast(lhs) | ValueCast(rhs)));
	}

	// If neither operand is _E::None, assigns (lhs | rhs) to lhs. The operands
	// are not interchangeable because (lhs |= _E::None) != (lhs | _E::None).
	// Only supports bitmask enum class types.
	template <typename _E,
		typename = std::enable_if_t<Internal::isScopedEnum_v<_E>>,
		typename = std::enable_if_t<std::is_same_v<GetEnumCategoryT<_E>, EnumBitmask>>>
	constexpr _E& operator|=(_E& lhs, const _E& rhs) noexcept 
	{
		if constexpr (HasNone<_E>())
		{
			if (!(!lhs || !rhs)) 
				lhs = EnumCast<_E>(static_cast<std::underlying_type_t<_E>>(ValueCast(lhs) | ValueCast(rhs)));
		}
		else
		{
			lhs = EnumCast<_E>(static_cast<std::underlying_type_t<_E>>(ValueCast(lhs) | ValueCast(rhs)));
		}

		return lhs;
	}

	// If neither operand is _E::None, returns the intersection of the two bit
	// masks. Otherwise returns _E::None. Only supports bitmask enum class types.
	template <typename _E,
		typename = std::enable_if_t<Internal::isScopedEnum_v<_E>>,
		typename = std::enable_if_t<std::is_same_v<GetEnumCategoryT<_E>, EnumBitmask>>>
	constexpr _E operator&(const _E& lhs, const _E& rhs) noexcept
	{
		if constexpr (HasNone<_E>())
			if (!lhs || !rhs)
				return GetNone<_E>();

		return EnumCast<_E>(static_cast<std::underlying_type_t<_E>>(ValueCast(lhs) & ValueCast(rhs)));
	}

	// If neither operand is _E::None, assigns (lhs & rhs) to lhs. The operands
	// are not interchangeable because (lhs &= _E::None) != (lhs & _E::None).
	// Only supports bitmask enum class types.
	template <typename _E,
		typename = std::enable_if_t<Internal::isScopedEnum_v<_E>>,
		typename = std::enable_if_t<std::is_same_v<GetEnumCategoryT<_E>, EnumBitmask>>>
	constexpr _E& operator&=(_E& lhs, const _E& rhs) noexcept
	{
		if constexpr (HasNone<_E>())
		{
			if (!(!lhs || !rhs))
				lhs = EnumCast<_E>(static_cast<std::underlying_type_t<_E>>(ValueCast(lhs) & ValueCast(rhs)));
		}
		else
		{
			lhs = EnumCast<_E>(static_cast<std::underlying_type_t<_E>>(ValueCast(lhs) & ValueCast(rhs)));
		}

		return lhs;
	}

	// If neither operand is _E::None, returns the symmetric difference of the
	// two bit masks. Otherwise returns _E::None. Only supports bitmask enum
	// class types.
	template <typename _E,
		typename = std::enable_if_t<Internal::isScopedEnum_v<_E>>,
		typename = std::enable_if_t<std::is_same_v<GetEnumCategoryT<_E>, EnumBitmask>>>
	constexpr _E operator^(const _E& lhs, const _E& rhs) noexcept
	{
		if constexpr (HasNone<_E>())
			if (!lhs || !rhs)
				return GetNone<_E>();

		return EnumCast<_E>(static_cast<std::underlying_type_t<_E>>(ValueCast(lhs) ^ ValueCast(rhs)));
	}

	// If neither operand is _E::None, assigns (lhs ^ rhs) to lhs. The operands
	// are not interchangeable because (lhs ^= _E::None) != (lhs ^ _E::None).
	// Only supports bitmask enum class types.
	template <typename _E,
		typename = std::enable_if_t<Internal::isScopedEnum_v<_E>>,
		typename = std::enable_if_t<std::is_same_v<GetEnumCategoryT<_E>, EnumBitmask>>>
	constexpr _E& operator^=(_E& lhs, const _E& rhs) noexcept
	{
		if constexpr (HasNone<_E>())
		{
			if (!(!lhs || !rhs))
				lhs = EnumCast<_E>(static_cast<std::underlying_type_t<_E>>(ValueCast(lhs) ^ ValueCast(rhs)));
		}
		else
		{
			lhs = EnumCast<_E>(static_cast<std::underlying_type_t<_E>>(ValueCast(lhs) ^ ValueCast(rhs)));
		}

		return lhs;
	}

	// If lhs is not equal to EnumT::None, toggles all bits in lhs. Otherwise
	// returns EnumT::None. Only supports bitmask enum class types.
	template <typename _E,
		typename = std::enable_if_t<Internal::isScopedEnum_v<_E>>,
		typename = std::enable_if_t<std::is_same_v<GetEnumCategoryT<_E>, EnumBitmask>>>
	constexpr _E operator~(const _E& lhs) noexcept
	{
		if constexpr (HasNone<_E>())
			if (!lhs) return GetNone<_E>();

		return EnumCast<_E>(~ValueCast(lhs));
	}

#pragma endregion

	// [Categorytraits]
	// Here we define our EnumCast Impl & Valid Impl!!!
#pragma region CategoryTraits
	template <>
	struct EnumCategoryTraits<EnumDefault>
	{
		using Category = EnumDefault;

		template <typename _E, typename = std::enable_if_t<std::is_same_v<GetEnumCategoryT<_E>, Category>>>
		static constexpr bool Valid() noexcept
		{
			return true;
		}

		template <typename _E, typename _VT, typename = std::enable_if_t<std::is_same_v<GetEnumCategoryT<_E>, Category>>>
		static constexpr _E EnumCast(_VT x) noexcept
		{
			return static_cast<_E>(x);
		}
	};

	template <>
	struct EnumCategoryTraits<EnumStandard>
	{
		using Category = EnumStandard;

		template <typename _E, typename = std::enable_if_t<std::is_same_v<GetEnumCategoryT<_E>, Category>>>
		static constexpr bool Valid() noexcept
		{
			if constexpr (HasNone<_E>())
			{
				constexpr auto xnone	= Internal::value_cast_impl(GetNone<_E>());
				constexpr auto xmin		= Internal::value_cast_impl(GetMin<_E>());
				constexpr auto xmax		= Internal::value_cast_impl(GetMax<_E>());
				return xmin <= xmax && xnone != Influx::Math::Clamp(xnone, xmin, xmax);
			}
			else return false;
		}

		template <typename _E, typename _VT, typename = std::enable_if_t<std::is_same_v<GetEnumCategoryT<_E>, Category>>>
		static constexpr _E EnumCast(_VT x) noexcept
		{
			constexpr auto xnone = Internal::value_cast_impl(GetNone<_E>());
			constexpr auto xmin = Internal::value_cast_impl(GetMin<_E>());
			constexpr auto xmax = Internal::value_cast_impl(GetMax<_E>());
			return static_cast<_E>(x == Influx::Math::Clamp(x, xmin, xmax) ? x : none);
		}
	};

	template <>
	struct EnumCategoryTraits<EnumLinear>
	{
		using Category = EnumLinear;

		template <typename _E, typename = std::enable_if_t<std::is_same_v<GetEnumCategoryT<_E>, Category>>>
		static constexpr bool Valid() noexcept
		{
			if constexpr (HasNone<_E>())
			{
				constexpr auto xnone = Internal::value_cast_impl(GetNone<_E>());
				constexpr auto xmin = Internal::value_cast_impl(GetMin<_E>());
				constexpr auto xmax = Internal::value_cast_impl(GetMax<_E>());
				return xmin <= xmax && xnone != Influx::Math::Clamp(xnone, xmin, xmax);
			}
			else return false;
		}

		template <typename _E, typename _VT, typename = std::enable_if_t<std::is_same_v<GetEnumCategoryT<_E>, Category>>>
		static constexpr _E EnumCast(_VT x) noexcept
		{
			constexpr auto xnone = Internal::value_cast_impl(GetNone<_E>());
			constexpr auto xmin = Internal::value_cast_impl(GetMin<_E>());
			constexpr auto xmax = Internal::value_cast_impl(GetMax<_E>());
			return static_cast<_E>(x == xnone ? xnone : Influx::Math::Clamp(x, xmin, xmax));
		}
	};

	template <>
	struct EnumCategoryTraits<EnumBitmask>
	{
		using Category = EnumBitmask;

		template <typename _E, typename = std::enable_if_t<std::is_same_v<GetEnumCategoryT<_E>, Category>>>
		static constexpr bool Valid() noexcept
		{
			constexpr auto xmin = Internal::value_cast_impl(GetMin<_E>());
			constexpr auto xmax = Internal::value_cast_impl(GetMax<_E>());
			if constexpr (std::is_unsigned_v<std::underlying_type_t<_E>> && (xmin & xmax) == xmin)
			{
				if constexpr (HasNone<_E>())
				{
					constexpr auto xnone = Internal::value_cast_impl(GetNone<_E>());
					return xnone != Influx::Math::ClampBitwise(xnone, xmin, xmax);
				}
				else return true;
			}
			else return false;
		}

		template <typename _E, typename _VT, typename = std::enable_if_t<std::is_same_v<GetEnumCategoryT<_E>, Category>>>
		static constexpr _E EnumCast(_VT x) noexcept
		{
			constexpr auto xmax = Internal::value_cast_impl(GetMax<_E>());
			constexpr auto xmin = Internal::value_cast_impl(GetMin<_E>());

			if constexpr (HasNone<_E>())
			{
				constexpr auto xnone = Internal::value_cast_impl(GetNone<_E>());
				return static_cast<_E>(x == xnone ? xnone : Influx::Math::ClampBitwise(x, xmin, xmax));
			}
			else return static_cast<_E>(Influx::Math::ClampBitwise(x, xmin, xmax));
		}
	};

	template <typename _E, _E... _V>
	struct EnumCategoryTraits<EnumDiscrete<_E, _V...>>
	{
		using Category = EnumDiscrete<_E, _V...>;

		template <typename _E_, typename = std::enable_if_t<std::is_same_v<GetEnumCategoryT<_E_>, Category>>>
		static constexpr bool Valid() noexcept
		{
			if constexpr (HasNone<_E_>())
			{
				return std::is_same_v<_E_, _E> && !(... || (_V == _E_::__CORE_ENUM_NAME_NONE));
			}
			else return false;
		}

		template <typename _E_, typename _VT,
			typename = std::enable_if_t<std::is_same_v<GetEnumCategoryT<_E_>, Category>>,
			typename = std::enable_if_t<std::is_convertible_v<_E_, _E>>>
			static constexpr _E EnumCast(_VT x) noexcept
		{
			if ((... || (Internal::value_cast_impl(_V) == x)))
				return static_cast<_E>(x);
			return _E::__CORE_ENUM_NAME_NONE;
		}
	};
#pragma endregion
}

#define FLX_ENUM_CLASS_WITH_CATEGORY(NAME, TYPE, CATEGORY, XNON, ...) \
enum class NAME : TYPE; \
template<> \
struct Influx::Enum::EnumTraits<NAME> { using Category = CATEGORY; }; \
enum class NAME : TYPE { \
	__VA_ARGS__, \
	__CORE_ENUM_NAME_MAX	= Influx::Math::Max(__VA_ARGS__), \
	__CORE_ENUM_NAME_MIN	= Influx::Math::Min(__VA_ARGS__), \
	__CORE_ENUM_NAME_NONE	= Influx::Math::Clamp(static_cast<TYPE>(XNON), static_cast<TYPE>(Influx::Math::Min(__VA_ARGS__)), static_cast<TYPE>(Influx::Math::Max(__VA_ARGS__))) \
	 };


#define FLX_ENUM_CLASS_STANDARD(NAME, TYPE, XNON, ...) FLX_ENUM_CLASS_WITH_CATEGORY(NAME, TYPE, Influx::Enum::EnumStandard,	XNON, __VA_ARGS__)
#define FLX_ENUM_CLASS_LINEAR(NAME, TYPE, XNON, ...)	FLX_ENUM_CLASS_WITH_CATEGORY(NAME, TYPE, Influx::Enum::EnumLinear,		XNON, __VA_ARGS__)
#define FLX_ENUM_CLASS_BITMASK(NAME, TYPE, XNON, ...)	FLX_ENUM_CLASS_WITH_CATEGORY(NAME, TYPE, Influx::Enum::EnumBitmask,		XNON, __VA_ARGS__)

#endif