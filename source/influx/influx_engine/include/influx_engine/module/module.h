#pragma once

#include "influx_engine.h"

namespace influx::engine
{
	class INFLUX_ENGINE_API base_module
	{
	public:
		virtual ~base_module() = default;

	protected:
		base_module() = default;
	};
}