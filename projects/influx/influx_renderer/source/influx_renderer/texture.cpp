#include "renderer_pch.h"

#include "influx_renderer/texture.h"
#include "influx_renderer/renderer_backend.h"
#include "influx_renderer/descriptor_manager.h"

#include "influx_graphics/device.h"
#include "influx_graphics/descriptorheap.h"

namespace influx::renderer
{
	// constructs a target from create_args, allocating new graphics resources
	texture::texture(graphics::device* device, const texture_create_args& args)
		: mp_device{ device }
		, m_args{ args }
		, m_current_dimensions{ args.m_width, args.m_heigth }
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

		// create srv:
		mp_srv = renderer_backend::get_descriptor_manager()->create_srv(mp_resource);
	}

	graphics::resource* texture::get_resource() const
	{
		return mp_resource;
	}

	graphics::shader_resource_view* texture::get_srv() const
	{
		return mp_srv;
	}

	uint32 texture::get_width() const
	{
		return m_current_dimensions.x;
	}

	uint32 texture::get_height() const
	{
		return m_current_dimensions.y;
	}

	uint32 texture::get_num_pixels() const
	{
		return get_width() * get_height();
	}

	uint32 texture::get_srv_heap_idx() const
	{
		return 0u;
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
		}
	}

	bool texture_data::is_valid() const
	{
		return true;
	}
}