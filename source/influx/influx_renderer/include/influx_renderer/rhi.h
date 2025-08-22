#pragma once

#define INFLUX_RENDER_BACKEND_RHI 0
#define INFLUX_RENDER_BACKEND_GRAPHICS 1

// influx::rhi
#if INFLUX_RENDER_BACKEND_RHI
#include "influx_rhi.h"
namespace influx::renderer
{
	using rhi_device = rhi::device;
	using rhi_commandlist = rhi::commandlist;
	using rhi_resource = rhi::resource;
	using rhi_descheap = rhi::descheap;
	using rhi_descriptor = rhi::descriptor;
	using rhi_descriptor_range = rhi::descriptor_range;
	using rhi_descheap_type = rhi::e_descriptor_heap_type;
	using rhi_bufferdesc = rhi::buffer_create_args;
	using rhi_texture2Ddesc = rhi::texture2D_create_args;
	using rhi_pixelformat = rhi::pixelformat;
	using rhi_resource_state = rhi::e_resource_state;
	using rhi_resource_bindflags = rhi::e_resource_bindflags;
	using rhi_store_op = rhi::e_store_op;
	using rhi_load_op = rhi::e_load_op;
	static constexpr uint32 k_num_descheap_types = rhi::k_num_descriptor_heap_types;
}
#endif
// influx::graphics
#if INFLUX_RENDER_BACKEND_GRAPHICS
#include "influx_graphics/device.h"
#include "influx_graphics/descriptors.h"
#include "influx_graphics/resource.h"
#include "influx_graphics/commandlist.h"
namespace influx::renderer
{
	using rhi_device = graphics::device;
	using rhi_commandlist = graphics::commandlist;
	using rhi_resource = graphics::resource;
	using rhi_descheap = graphics::descriptor_heap;
	using rhi_descriptor = graphics::descriptor_handle;
	using rhi_descriptor_range = graphics::descriptor_range;
	using rhi_descheap_type = graphics::e_descriptor_heap_type;
	using rhi_bufferdesc = graphics::buffer_desc;
	using rhi_texture2Ddesc = graphics::tex2D_desc;
	using rhi_pixelformat = graphics::e_format;
	using rhi_resource_state = graphics::e_resource_state;
	using rhi_resource_bindflags = graphics::e_bind_flags;
	using rhi_store_op = graphics::e_store_op;
	using rhi_load_op = graphics::e_load_op;
	static constexpr uint32 k_num_descheap_types = graphics::k_num_descriptor_heap_types;
}
#endif