#pragma once

namespace influx
{
	class game_api final
	{
	public:
		void (*log)(const char* msg);
	};

	class plugin_api final
	{
	public:
		void (*log)(const char* msg);
	};
}
