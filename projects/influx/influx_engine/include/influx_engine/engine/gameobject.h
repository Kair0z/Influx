#pragma once

// influx::engine
#include "influx_engine/engine/engine.h"

namespace influx::engine
{
	class gameobject
	{
	public:
		template <typename _c, typename ..._init>
		_c* add_component(_init&&...);

		template <typename _c>
		void remove_component();

		template <typename _c>
		_c* get_component() const;

		template <typename _c>
		bool has_component() const;
	};

	namespace detail
	{
		template <typename _c, typename ..._init>
		_c* create_component(_init&&... init)
		{
			return nullptr;
		}
	}

	template <typename _c, typename ..._init>
	inline _c* gameobject::add_component(_init&&... init)
	{
		return detail::create_component<_c>(init...);
	}
}