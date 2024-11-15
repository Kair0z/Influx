#include "renderer_pch.h"

// influx::renderer
#include "influx_renderer/target.h"
#include "influx_renderer/renderer_backend.h"
#include "influx_renderer/descriptor_manager.h"

// influx::graphics
#include "influx_graphics/device.h"
#include "influx_graphics/descriptors.h"

namespace influx::renderer
{
	// constructs a target from create_args, allocating new graphics resources
	target::target(graphics::device* device, const target_create_args& args)
		: mp_device{device}
		, m_args{args}
	{
		influx_assert(args.m_has_colour || args.m_has_depth_stencil);

		if (args.m_has_colour)
		{
			// create the colour resource
			graphics::tex2D_desc desc{};
			desc.m_arraysize = 1u;
			desc.m_dimensions = { args.m_width, args.m_heigth };
			desc.m_format = graphics::e_format::rgba8;
			desc.m_num_mips = 1u;
			desc.m_sample_count = 1u;
			desc.m_flags = graphics::e_resource_flags::render_target;
			desc.m_init_state = graphics::e_resource_state::render_target;
			mp_resource = device->create_resource(desc);

			mp_rtv = renderer_backend::get_descriptor_manager()->create_rtv(mp_resource);
			m_rtv_handle = mp_rtv->get_cpu_handle();
		}
		if (args.m_has_depth_stencil)
		{
			graphics::tex2D_desc desc{};
			desc.m_arraysize = 1u;
			desc.m_dimensions = { args.m_width, args.m_heigth };
			desc.m_format = graphics::e_format::d32;
			desc.m_num_mips = 1u;
			desc.m_sample_count = 1u;
			desc.m_flags = graphics::e_resource_flags::depth_stencil;
			desc.m_init_state = graphics::e_resource_state::depth_write;
			mp_depth_resource = device->create_resource(desc);

			mp_dsv = renderer_backend::get_descriptor_manager()->create_dsv(mp_depth_resource);
			m_dsv_handle = mp_dsv->get_cpu_handle();
		}

		m_current_dimensions = { args.m_width, args.m_heigth };
	}

	// constructs a target from existing swapchain resources
	target::target(graphics::device* device, 
		graphics::swapchain* swapchain, 
		uint8 swapchain_index)
	{
		influx_assert(swapchain_index < swapchain->get_num_backbuffers());

		// get the existing backbuffer resource, and allocate + create a new rtv
		mp_resource = swapchain->get_backbuffer_resource(swapchain_index);
		mp_rtv = renderer_backend::get_descriptor_manager()->create_rtv(mp_resource);
		mp_dsv = nullptr;

		m_args.m_width = swapchain->get_dimensions().x;
		m_args.m_heigth = swapchain->get_dimensions().y;
		m_current_dimensions = swapchain->get_dimensions();

		// store the descriptor handles
		m_rtv_handle = mp_rtv->get_cpu_handle();
		m_dsv_handle = nullptr;

		mp_device = device;
	}

	target::~target()
	{
		descriptor_manager* desc_man = renderer_backend::get_descriptor_manager();
		if (mp_rtv)
		{
			desc_man->cleanup_rtv(mp_rtv);
			mp_rtv = nullptr;
		}

		if (mp_dsv)
		{
			desc_man->cleanup_dsv(mp_dsv);
			mp_dsv = nullptr;
		}
	}

	graphics::resource* target::get_resource() const
	{
		return mp_resource;
	}

	graphics::render_target_view* target::get_rtv() const
	{
		return mp_rtv;
	}

	graphics::depth_stencil_view* target::get_dsv() const
	{
		return mp_dsv;
	}

	uint32 target::get_width() const
	{
		return m_current_dimensions.x;
	}

	uint32 target::get_height() const
	{
		return m_current_dimensions.y;
	}

	bool target::has_depth_stencil() const
	{
		return mp_depth_resource != nullptr;
	}

	bool target::is_depth_only() const
	{
		return !m_args.m_has_colour && m_args.m_has_depth_stencil;
	}

	void target::resize(const math::vectoru2& dimensions)
	{
		if (dimensions != m_current_dimensions)
		{
			delete mp_resource;

			// update size:
			m_current_dimensions = dimensions;

			if (m_args.m_has_colour)
			{
				// create new resource
				graphics::tex2D_desc desc{};
				desc.m_arraysize = 1u;
				desc.m_dimensions = { dimensions.x, dimensions.y };
				desc.m_flags;
				desc.m_format = graphics::e_format::rgba8;
				desc.m_num_mips = 1u;
				desc.m_sample_count = 1u;
				mp_resource = mp_device->create_resource(desc);

				recreate_rtv();
			}

			if (m_args.m_has_depth_stencil)
			{
				graphics::tex2D_desc desc{};
				desc.m_arraysize = 1u;
				desc.m_dimensions = { dimensions.x, dimensions.y };
				desc.m_format = graphics::e_format::d32;
				desc.m_num_mips = 1u;
				desc.m_sample_count = 1u;
				desc.m_flags = graphics::e_resource_flags::depth_stencil;
				desc.m_init_state = graphics::e_resource_state::depth_write;
				mp_depth_resource = mp_device->create_resource(desc);

				recreate_dsv();
			}
		}
	}

	void target::recreate_rtv()
	{
		if (mp_rtv)
		{
			delete mp_rtv;
			mp_rtv = mp_device->create_rtv(m_rtv_handle, mp_resource);
		}
	}

	void target::recreate_dsv()
	{
		if (mp_dsv)
		{
			delete mp_dsv;
			mp_dsv = mp_device->create_dsv(m_dsv_handle, mp_resource);
		}
	}

	void target::set_name(const debug_name& name)
	{
		m_debug_name = name;

		if (mp_resource && mp_resource->is_valid())
		{
			mp_resource->set_name(m_debug_name);
		}

		if (mp_depth_resource && mp_depth_resource->is_valid())
		{
			mp_depth_resource->set_name(m_debug_name);
		}
	}

	const debug_name& target::get_name() const
	{
		return m_debug_name;
	}
}

