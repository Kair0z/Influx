#pragma once

namespace influx
{
	struct plugin_load_args final
	{

	};

	class plugin_interface
	{
	public:
		virtual void load(const plugin_load_args& args) = 0;
		virtual void unload() = 0;
	};
}