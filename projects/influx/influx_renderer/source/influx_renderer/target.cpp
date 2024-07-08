#include "renderer_pch.h"
#include "influx_renderer/target.h"

#include "influx_graphics/device.h"
#include "influx_graphics/descriptorheap.h"

namespace influx::renderer
{
	// constructs a target from create_args, allocating new graphics resources
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

		// allocate & create the rtv
		mp_rtv = device->create_rtv(rtv_heap, mp_resource);

		target(device, mp_resource, mp_rtv);
	}

	// constructs a target from existing swapchain resources
	target::target(graphics::device* device, graphics::swapchain* swapchain, uint8 swapchain_index, graphics::descriptor_heap* rtv_heap)
	{
		influx_assert(swapchain_index < swapchain->get_num_backbuffers());

		// get the existing backbuffer resource, and allocate + create a new rtv
		mp_resource = swapchain->get_backbuffer_resource(swapchain_index);
		mp_rtv = device->create_rtv(rtv_heap, mp_resource);

		m_args.m_width = swapchain->get_dimensions().x;
		m_args.m_heigth = swapchain->get_dimensions().y;
		m_current_dimensions = swapchain->get_dimensions();

		// store the descriptor handle
		m_descriptor_handle = mp_rtv->get_descriptor_handle();

		mp_device = device;
	}

	target::target(graphics::device* device, graphics::resource* resource, graphics::render_target_view* rtv)
		: mp_device{device}
	{
		m_args.m_width = resource->get_width();
		m_args.m_heigth = resource->get_height();
		m_current_dimensions = { m_args.m_width, m_args.m_heigth };
		mp_resource = resource;
		mp_rtv = rtv;

		// store the descriptor handle
		m_descriptor_handle = mp_rtv->get_descriptor_handle();
	}

	graphics::resource* target::get_resource() const
	{
		return mp_resource;
	}

	graphics::render_target_view* target::get_rtv() const
	{
		return mp_rtv;
	}

	uint32 target::get_width() const
	{
		return m_current_dimensions.x;
	}

	uint32 target::get_height() const
	{
		return m_current_dimensions.y;
	}

	void target::resize(const math::vectoru2& dimensions)
	{
		if (dimensions != m_current_dimensions)
		{
			delete mp_resource;

			// update size:
			m_current_dimensions = dimensions;

			// create new resource
			graphics::tex2D_desc desc{};
			desc.m_arraysize = 1u;
			desc.m_dimensions = { dimensions.x, dimensions.y };
			desc.m_flags;
			desc.m_format = graphics::e_format::rgba8;
			desc.m_num_mips = 1u;
			desc.m_sample_count = 1u;
			mp_resource = mp_device->create_resource(desc);

			// recreate our rtv
			recreate_rtv();
		}
	}

	void target::recreate_rtv()
	{
		delete mp_rtv;
		mp_rtv = mp_device->create_rtv(m_descriptor_handle, mp_resource);
	}

#if _DEBUG
	void target::set_name(const string& name)
	{
		m_debug_name = name;

#if _DEBUG
		mp_resource->set_name(m_debug_name);
#endif
	}

	const string& target::get_name() const
	{
		return m_debug_name;
	}
#endif
}

