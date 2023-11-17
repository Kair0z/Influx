#pragma once

#include "threads.h"

namespace influx::application
{
	class gamethread final : public dedicated_thread
	{
	public:
		def_inherit_static_void_func(initialize());
		def_inherit_static_void_func(tick());
		def_inherit_static_void_func(cleanup());
	};
}