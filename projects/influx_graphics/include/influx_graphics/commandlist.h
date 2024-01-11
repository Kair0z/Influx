#pragma once
#include "influx_graphics/base.h"

namespace influx::graphics
{
	class command_allocator;
	class pipeline_state;

	class command_list : public base
	{
	public:
		virtual void start(command_allocator* allocator, pipeline_state* init_state = nullptr) = 0;
		
		virtual void end() = 0;
	};
}