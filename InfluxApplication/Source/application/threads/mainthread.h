#pragma once
#include "threads.h"

namespace influx::application
{
	class mainthread final : public dedicated_thread
	{
	public:
		def_inherit_static_void_func(initialize());
		def_inherit_static_void_func(tick());
		def_inherit_static_void_func(cleanup());
	};
}