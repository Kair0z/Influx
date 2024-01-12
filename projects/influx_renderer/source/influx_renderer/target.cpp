#include "renderer_pch.h"
#include "influx_renderer/target.h"

#include "influx_graphics/device.h"
#include "influx_graphics/descriptorheap.h"

namespace influx::renderer
{
	target::target(graphics::device* device, graphics::descriptor_heap* rtv_heap, const target_create_args& args)
	{
		// create the resource
		graphics::tex2D_desc desc{};
		desc.m_arraysize = 1u;
		desc.m_dimensions = { args.m_width, args.m_heigth };
		desc.m_flags;
		desc.m_format = graphics::e_format::rgba8;
		desc.m_num_mips = 1u;
		desc.m_sample_count = 1u;

		// create the underlying resource
		mp_resource = device->create_resource(desc);

		// create the rtv
		mp_rtv = device->create_rtv(rtv_heap, mp_resource);

		target(mp_resource, mp_rtv);
	}

	target::target(graphics::resource* resource, graphics::render_target_view* rtv)
	{
		m_args.m_width = resource->get_width();
		m_args.m_heigth = resource->get_height();
		mp_resource = resource;
		mp_rtv = rtv;
	}

	graphics::resource* target::get_resource() const
	{
		return mp_resource;
	}

	graphics::render_target_view* target::get_rtv() const
	{
		return mp_rtv;
	}

	vector<target*> target::create_swapchain_targets(graphics::device* device, 
		graphics::swapchain* swapchain, graphics::descriptor_heap* rtv_heap)
	{
		vector<target*> targets{};

		for (uint8 i = 0u; i < swapchain->get_num_backbuffers(); ++i)
		{
			graphics::resource* backbuffer = swapchain->get_backbuffer_resource(i);
			target* new_target = new target(backbuffer, device->create_rtv(rtv_heap, backbuffer));

			targets.push_back(new_target);
		}

		return targets;
	}
}

