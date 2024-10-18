#pragma once
#include <variant>

namespace influx::events
{
	namespace detail
	{
		class ievent
		{
			
		};
	}

	template <typename ..._types>
	class event final : public detail::ievent
	{
	public:

	private:
		std::variant<_types...> m_variant_data;
	};
}