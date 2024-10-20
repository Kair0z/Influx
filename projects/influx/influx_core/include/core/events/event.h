#pragma once
#include <variant>

namespace influx::events
{
	namespace detail
	{
		class ievent
		{
			
		};

		// Helper to check if a type exists in the variadic template arguments
		template <typename T, typename... Args>
		struct is_in_pack;

		// Specialization: If the first argument matches T, we found it!
		template <typename T, typename First, typename... Rest>
		struct is_in_pack<T, First, Rest...> : is_in_pack<T, Rest...> {};

		// Base case: T matches First
		template <typename T, typename... Rest>
		struct is_in_pack<T, T, Rest...> : std::true_type {};

		// If no arguments left in the pack
		template <typename T>
		struct is_in_pack<T> : std::false_type {};
	}

	template <typename ..._types>
	class event final : public detail::ievent
	{
	public:
		template <typename _evtype>
		_evtype& get()
		{
			static_assert(can_be<_evtype>(), "type _evtype is not part of this variadic event type");
			return std::get<_evtype>(m_variant_data);
		}

		template <typename _evtype>
		const _evtype& get() const
		{
			static_assert(can_be<_evtype>(), "type _evtype is not part of this variadic event type");
			return std::get<_evtype>(m_variant_data);
		}

		template <typename _evtype>
		_evtype const* get_if() const
		{
			static_assert(can_be<_evtype>(), "type _evtype is not part of this variadic event type");
			return std::get_if<_evtype>(&m_variant_data);
		}

		template <typename _evtype>
		_evtype* get_if()
		{
			static_assert(can_be<_evtype>(), "type _evtype is not part of this variadic event type");
			return std::get_if<_evtype>(&m_variant_data);
		}

		template <typename _evtype, class... _args>
		void set(_args&&... args)
		{
			static_assert(can_be<_evtype>(), "type _evtype is not part of this variadic event type");
			m_variant_data.emplace<_evtype>(std::forward<_args>(args)...);
		}

		template <typename _evtype>
		static constexpr bool can_be()
		{
			return detail::is_in_pack<_evtype, _types...>::value;
		}

	private:
		std::variant<_types...> m_variant_data;
	};
}