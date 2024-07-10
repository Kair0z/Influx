#include "renderer_pch.h"
#include "influx_renderer/texture.h"

#include "influx_graphics/device.h"
#include "influx_graphics/descriptorheap.h"

namespace influx::renderer
{
	// constructs a target from create_args, allocating new graphics resources
	texture::texture(graphics::device* device, graphics::descriptor_heap* irv_heap, const texture_create_args& args)
	{
		// create the resource
		graphics::tex2D_desc desc{};
		desc.m_arraysize = 1u;
		desc.m_dimensions = { args.m_width, args.m_heigth };
		desc.m_flags;
		desc.m_format = graphics::e_format::rgba8;
		desc.m_num_mips = 1u;
		desc.m_sample_count = 1u;
		desc.m_init_state = graphics::e_resource_state::shader_resource;

		// create the underlying resource
		mp_resource = device->create_resource(desc);

		// allocate & create the rtv
		mp_irv = device->create_srv(irv_heap, mp_resource);

		texture(device, mp_resource, mp_irv);
	}

	texture::texture(graphics::device* device, graphics::resource* resource, graphics::input_resource_view* irv)
	{
		m_args.m_width = resource->get_width();
		m_args.m_heigth = resource->get_height();
		m_current_dimensions = { m_args.m_width, m_args.m_heigth };
		mp_resource = resource;
		mp_irv = irv;

		// store the descriptor handles
		m_cpu_handle = mp_irv->get_cpu_handle();
		m_gpu_handle = mp_irv->get_gpu_handle();
	}


	graphics::resource* texture::get_resource() const
	{
		return mp_resource;
	}

	graphics::input_resource_view* texture::get_irv() const
	{
		return mp_irv;
	}

	uint32 texture::get_width() const
	{
		return m_current_dimensions.x;
	}

	uint32 texture::get_height() const
	{
		return m_current_dimensions.y;
	}

#if _DEBUG
	void texture::set_name(const string& name)
	{
		m_debug_name = name;
	}

	const string& texture::get_name() const
	{
		return m_debug_name;
	}
#endif

	void texture::resize(const math::vectoru2& new_dimensions)
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
			desc.m_flags;
			desc.m_format = graphics::e_format::rgba8;
			desc.m_num_mips = 1u;
			desc.m_sample_count = 1u;
			mp_resource = mp_device->create_resource(desc);

			// recreate our irv
			// ...
		}
	}

	bool texture_data::is_valid() const
	{
		return true;
	}
}