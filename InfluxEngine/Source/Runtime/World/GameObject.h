#pragma once
#include "Runtime/Entity/EntityManager.h"

namespace Influx
{
	class GameObject final
	{
	public:
		GameObject() = default;
		Entity* mpEntity;
	};
}


