#pragma once

namespace influx
{
	struct engine_api final
	{
		void (*log)(const char* msg);
	};
}
