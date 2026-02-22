#pragma once

#include "debug.h"
#include <type_traits>
#include <concepts>
#include <bit>
#include <utility>

#include <cstddef>
#include <type_traits>
#include <array>
#include <algorithm>
#include <string_view>
#include <stdexcept>
#include <concepts>


namespace influx
{
    using uint64 = size_t;

    template <typename _e>
    constexpr std::underlying_type_t<_e> to_underlying(_e e) noexcept {
        static_assert(std::is_enum_v<_e>, "to_underlying requires an enum type");
        return static_cast<std::underlying_type_t<_e>>(e);
    }

#pragma region metadata
    /** A object that holds enum-values and strings.
    *
    * @tparam ValueType The enum-type.
    * @tparam NameType The type used to convert to and from the EnumType.
    * @tparam N Number of enum-values.
    */
    
    template<typename _e, typename _str, uint64 _n>
    class enum_metadata 
    {
    public:
        using enum_type = _e;
        using name_type = _str;
        static_assert(std::is_enum_v<enum_type>, "enum_type must be an enum");
        static_assert(_n != 0);
        

        /** The number of enum cvalues.
         */
        constexpr static uint64 k_count = _n;

        /** The numeric values in the enum do not contain a gap.
         */
        bool k_values_are_continuous;

        /** Get the number of enum values.
         */
        [[nodiscard]] constexpr uint64 size() const noexcept
        {
            return k_count;
        }

        /** Get the minimum value.
         */
        [[nodiscard]] constexpr enum_type minimum() const noexcept
        {
            return std::get<0>(_by_value).value;
        }

        /** Get the maximum value.
         */
        [[nodiscard]] constexpr enum_type maximum() const noexcept
        {
            return std::get<k_count - 1u>(_by_value).value;
        }

        /** Construct a enum-names table object.
         *
         * Example usage:
         * ```
         * enum class my_bool { yes, no };
         * constexpr auto my_bool_names = enum_metadata(my_bool::no, "no", my_bool::yes, "yes");
         * ```
         *
         * The template parameters of the class will be deduced from the
         * constructor. `N = sizeof...(Args) / 2`, `T = decltype(args[0])`.
         *
         * @param args A list of a enum-value and names.
         */
        template<typename... _args>
        [[nodiscard]] constexpr enum_metadata(_args const&...args) noexcept
        {
            static_assert(sizeof...(_args) == k_count * 2u);
            add_value_name<0>(args...);

            std::sort(_by_name.begin(), _by_name.end(), [](auto const& a, auto const& b) {
                return a.name < b.name;
                });

            std::sort(_by_value.begin(), _by_value.end(), [](auto const& a, auto const& b) {
                return to_underlying(a.value) < to_underlying(b.value);
                });

            k_values_are_continuous = check_values_are_continuous();
        }

        /** Check if the enum has a name.
         *
         * @param name The name to lookup in the enum.
         * @return True if the name is found.
         */
        template<std::convertible_to<name_type> _name>
        [[nodiscard]] constexpr bool contains(_name&& name) const noexcept
        {
            return find(name_type{ std::forward<_name>(name) }) != nullptr;
        }

        /** Check if the enum has a value.
         *
         * @param value The value to lookup for the enum.
         * @return True if the value is found.
         */
        [[nodiscard]] constexpr bool contains(enum_type value) const noexcept
        {
            return find(value) != nullptr;
        }

        /** Get an enum-value from a name.
         *
         * @param name The name to lookup in the enum.
         * @return The enum-value belonging with the name.
         * @throws std::out_of_range When the name does not exist.
         */
        template<std::convertible_to<name_type> _name>
        [[nodiscard]] constexpr enum_type at(_name&& name) const
        {
            if (auto const* value = find(name_type{ std::forward<_name>(name) })) {
                return *value;
            }
            else {
                throw std::out_of_range{ "enum_metadata::at" };
            }
        }

        /** Get a name from an enum-value.
         *
         * @param value The enum value to lookup.
         * @return The name belonging with the enum value.
         * @throws std::out_of_range When the value does not exist.
         */
        [[nodiscard]] constexpr name_type const& at(enum_type value) const
        {
            if (auto const* name = find(value)) {
                return *name;
            }
            else {
                throw std::out_of_range{ "enum_metadata::at" };
            }
        }

        /** Get an enum-value from a name.
         *
         * @param name The name to lookup in the enum.
         * @return The enum-value belonging with the name or std::nullopt if name is not found.
         */
        template<std::convertible_to<name_type> _name>
        [[nodiscard]] constexpr std::optional<enum_type> at_if(_name&& name) const noexcept
        {
            if (auto const* value = find(name_type{ std::forward<_name>(name) })) {
                return *value;
            }
            else {
                return std::nullopt;
            }
        }

        /** Get an enum-value from a name.
         *
         * @param name The name to lookup in the enum.
         * @param default_value The default value to return when the name is not found.
         * @return The enum-value belonging with the name.
         */
        template<std::convertible_to<name_type> _name>
        [[nodiscard]] constexpr enum_type at(_name&& name, enum_type default_value) const noexcept
        {
            if (auto const* value = find(name_type{ std::forward<_name>(name) })) {
                return *value;
            }
            else {
                return default_value;
            }
        }

        /** Get a name from an enum-value.
         *
         * @param value The enum value to lookup.
         * @param default_name The default name to return when value is not found.
         * @return The name belonging with the enum value.
         */
        template<std::convertible_to<name_type> _name>
        [[nodiscard]] constexpr name_type at(enum_type value, _name&& default_name) const noexcept
        {
            if (auto const* name = find(value)) {
                return *name;
            }
            else {
                return std::forward<_name>(default_name);
            }
        }

        /** Get an enum-value from a name.
         *
         * @note It is undefined-behavior to lookup a name that does not exist in the table.
         * @param name The name to lookup in the enum.
         * @return The enum-value belonging with the name.
         */
        template<std::convertible_to<name_type> _name>
        [[nodiscard]] constexpr enum_type operator[](_name&& name) const noexcept
        {
            auto* value = find(name_type{ std::forward<_name>(name) });
            // hi_assert_not_null(value);
            return *value;
        }

        /** Get a name from an enum-value.
         *
         * @note It is undefined-behavior to lookup a value that does not exist in the table.
         * @param value The enum value to lookup.
         * @return The name belonging with the enum value.
         */
        [[nodiscard]] constexpr name_type const& operator[](enum_type value) const noexcept
        {
            auto* name = find(value);
            // hi_assert_not_null(name);
            return *name;
        }

    private:
        struct value_name {
            enum_type value;
            name_type name;

            constexpr value_name() noexcept : value(), name() {}
            constexpr value_name(enum_type value, name_type name) noexcept : value(value), name(std::move(name)) {}
        };

        std::array<value_name, k_count> _by_name;
        std::array<value_name, k_count> _by_value;

        [[nodiscard]] constexpr name_type const* find(enum_type value) const noexcept
        {
            if (k_values_are_continuous) {
                // If the enum values are continues we can do an associative lookup.
                auto const it = _by_value.begin();
                auto const offset = to_underlying(it->value);
                auto const i = to_underlying(value) - offset;
                return (i >= 0 and i < _n) ? &(it + i)->name : nullptr;

            }
            else {
                auto const it = std::lower_bound(_by_value.begin(), _by_value.end(), value, [](auto const& item, auto const& key) {
                    return item.value < key;
                    });

                return (it != _by_value.end() and it->value == value) ? &it->name : nullptr;
            }
        }

        [[nodiscard]] constexpr enum_type const* find(name_type const& name) const noexcept
        {
            auto const it = std::lower_bound(_by_name.begin(), _by_name.end(), name, [](auto const& item, auto const& key) {
                return item.name < key;
                });

            return (it != _by_name.end() and it->name == name) ? &it->value : nullptr;
        }

        /** Add value and name to table.
         *
         * Used by the constructor.
         */
        template<uint64 _i, typename... _rest>
        constexpr void add_value_name(enum_type value, name_type name, _rest const&...rest) noexcept
        {
            static_assert(sizeof...(_rest) % 2 == 0);

            std::get<_i>(_by_name) = { value, name };
            std::get<_i>(_by_value) = { value, std::move(name) };

            if constexpr (sizeof...(_rest) > 0) {
                add_value_name<_i + 1>(rest...);
            }
        }

        /** Check if the values are continues
         *
         * Used by the constructor.
         */
        [[nodiscard]] constexpr bool check_values_are_continuous() const noexcept
        {
            auto check_value = to_underlying(minimum());
            for (auto const& item : _by_value) {
                if (to_underlying(item.value) != check_value++) {
                    return false;
                }
            }
            return true;
        }
    };

    template<typename T>
    struct enum_metadata_name {
        using type = std::decay_t<T>;
    };

    // clang-format off
    template<> struct enum_metadata_name<char const*> { using type = std::string_view; };
    template<> struct enum_metadata_name<char*> { using type = std::string_view; };
    template<size_t N> struct enum_metadata_name<char[N]> { using type = std::string_view; };
    template<size_t N> struct enum_metadata_name<char const [N]> { using type = std::string_view; };
    // clang-format on

    template<typename T>
    using enum_metadata_name_t = enum_metadata_name<T>::type;

    template<typename ValueType, typename NameType, typename... Rest>
    enum_metadata(ValueType const&, NameType const&, Rest const&...)
        -> enum_metadata<ValueType, enum_metadata_name_t<NameType>, (sizeof...(Rest) + 2) / 2>;
#pragma endregion

#pragma region bitmask operators
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
	inline typename std::enable_if_t<enable_bitmask_operators<E>, E>::type operator&(const E& lhs, const E& rhs)
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
#pragma endregion
}

#define ENABLE_ENUM_BIT_OPERATORS(type) \
	template<> struct influx::enum_bitmask_operators<type> { static constexpr inline bool enable = true; }