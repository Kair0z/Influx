#include "rendergraph_pch.h"
#include "rendergraph.h"

#include <iostream>

namespace influx::rendergraph
{
	void plugin::load(const plugin_load_args& args)
	{
		std::cout << "rendergraph::plugin::load \n";
	}
	void plugin::unload()
	{

	}
}
