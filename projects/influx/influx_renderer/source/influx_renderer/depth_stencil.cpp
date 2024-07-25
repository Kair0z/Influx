#include "renderer_pch.h"
#include "influx_renderer/depth_stencil.h"

#include "influx_graphics/device.h"
#include "influx_graphics/descriptorheap.h"

namespace influx::renderer
{
	// constructs a target from create_args, allocating new graphics resources
	depth_stencil::depth_stencil(graphics::device* device, graphics::descriptor_heap* dsv_heap, const depth_stencil_create_args& args)
	{
		// create the resource
		graphics::tex2D_desc desc{};
		desc.m_arraysize = 1u;
		desc.m_dimensions = { args.m_width, args.m_heigth };
		desc.m_flags = graphics::e_resource_flags::depth_stencil;
		desc.m_format = graphics::e_format::d32;
		desc.m_num_mips = 1u;
		desc.m_sample_count = 1u;
		desc.m_init_state = graphics::e_resource_state::depth_write;

		// create the underlying resource
		mp_resource = device->create_resource(desc);

		// allocate & create the rtv
		mp_dsv = device->create_dsv(dsv_heap, mp_resource);

		depth_stencil(device, mp_resource, mp_dsv);
	}

	depth_stencil::depth_stencil(graphics::device* device, graphics::resource* resource, graphics::depth_stencil_view* dsv)
		: mp_device{ device }
	{
		m_args.m_width = resource->get_width();
		m_args.m_heigth = resource->get_height();
		m_current_dimensions = { m_args.m_width, m_args.m_heigth };
		mp_resource = resource;
		mp_dsv = dsv;

		// store the descriptor handle
		m_descriptor_handle = mp_dsv->get_cpu_handle();
	}

	void depth_stencil::resize(const math::vectoru2& new_dimensions)
	{
		if (new_dimensions != m_current_dimensions)
		{
			delete mp_resource;

			// update size:
			m_current_dimensions = new_dimensions;

			// create new resource
			graphics::tex2D_desc desc{};
			desc.m_arraysize = 1u;
			desc.m_dimensions = { new_dimensions.x, new_dimensions.y };
			desc.m_flags = graphics::e_resource_flags::depth_stencil;
			desc.m_format = graphics::e_format::d32;
			desc.m_num_mips = 1u;
			desc.m_sample_count = 1u;
			desc.m_init_state = graphics::e_resource_state::depth_write;

			mp_resource = mp_device->create_resource(desc);

			// recreate our dsv
			recreate_dsv();
		}
	}

	void depth_stencil::recreate_dsv()
	{
		delete mp_dsv;
		mp_dsv = mp_device->create_dsv(m_descriptor_handle, mp_resource);
	}
	
	graphics::resource* depth_stencil::get_resource() const
	{
		return mp_resource;
	}

	graphics::depth_stencil_view* depth_stencil::get_dsv() const
	{
		return mp_dsv;
	}

	uint32 depth_stencil::get_width() const
	{
		return m_current_dimensions.x;
	}

	uint32 depth_stencil::get_height() const
	{
		return m_current_dimensions.y;
	}

// should we change it to be for profile as well?
#if _DEBUG
	void depth_stencil::set_name(const string& name)
	{
		m_debug_name = name;
#if _DEBUG
		mp_resource->set_name(m_debug_name);
#endif
	}

	const string& depth_stencil::get_name() const
	{
		return m_debug_name;
	}
#endif

}