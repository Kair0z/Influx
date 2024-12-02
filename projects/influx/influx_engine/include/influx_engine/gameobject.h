#pragma once

// influx::core
#include "core/basetypes.h"

// influx::engine
#include "influx_engine/layer/layer.h"

namespace influx::engine
{
	class gameobject
	{
	public:
		template <typename _ctype, typename ..._init>
		_ctype* add_component(_init&&...);

		uint32 get_id() const;

	private:
		layer* m_owner;
		friend class layer;

		uint32 m_id;
	};

	template <typename _ctype, typename ..._init>
	inline _ctype* gameobject::add_component(_init&&... init)
	{
		return m_owner->create_component<_ctype>(get_id(), init...);
	}
}