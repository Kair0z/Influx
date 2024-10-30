#include "graphics_pch.h"
#include "influx_graphics/commandlist.h"

#include "influx_graphics/device.h"

namespace influx::graphics
{
	commandlist* commandlist::create(device* device, command_allocator* allocator, pipeline* init_pipeline)
	{
		influx_assert(device != nullptr);
		return device->create_graphics_command_list(allocator, init_pipeline);
	}
}