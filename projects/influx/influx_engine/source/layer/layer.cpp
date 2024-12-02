#include "engine_pch.h"
#include "influx_engine/layer/layer.h"

namespace influx::engine
{
	gameobject* layer::create()
	{
		gameobject* new_object = new gameobject();
		new_object->m_owner = this;
		new_object->m_id = get_engine()->get_world()->create_entity();
		return new_object;
	}
}

