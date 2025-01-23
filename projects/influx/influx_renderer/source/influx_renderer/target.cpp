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

		descriptor_manager& desc_manager = *renderer_backend::get_descriptor_manager();
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

			m_rtv_cpu = desc_manager.create_rtv(mp_resource);
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
			desc.m_init_state = graphics::e_resource_state::depth_target;
			mp_depth_resource = device->create_resource(desc);

			m_dsv_cpu = desc_manager.create_dsv(mp_depth_resource);
		}

		m_current_dimensions = { args.m_width, args.m_heigth };
		m_is_swapchain_target = false;
	}

	// constructs a target from existing swapchain resources
	target::target(graphics::device* device, 
		graphics::swapchain* swapchain, 
		uint8 swapchain_index)
	{
		influx_assert(swapchain_index < swapchain->get_num_backbuffers());

		// get the existing backbuffer resource, and allocate + create a new rtv
		mp_resource = swapchain->get_backbuffer_resource(swapchain_index);
		m_rtv_cpu = renderer_backend::get_descriptor_manager()->create_rtv(mp_resource);
		m_dsv_cpu = nullptr;

		m_args.m_width = swapchain->get_dimensions().x;
		m_args.m_heigth = swapchain->get_dimensions().y;
		m_current_dimensions = swapchain->get_dimensions();

		mp_device = device;

		m_is_swapchain_target = true;
	}

	target::~target()
	{
		descriptor_manager* desc_man = renderer_backend::get_descriptor_manager();
		if (m_rtv_cpu)
		{
			desc_man->cleanup_rtv(m_rtv_cpu);
			m_rtv_cpu = nullptr;
		}
		if (m_dsv_cpu)
		{
			desc_man->cleanup_dsv(m_dsv_cpu);
			m_dsv_cpu = nullptr;
		}

		// all non-swapchain targets own their own resources and so should destroy them
		if (m_is_swapchain_target == false)
		{
			if (mp_resource)
			{
				mp_device->release(mp_resource);
				mp_resource = nullptr;
			}

			if (mp_depth_resource)
			{
				mp_device->release(mp_depth_resource);
				mp_depth_resource = nullptr;
			}
		}
	}

	graphics::resource* target::get_resource() const
	{
		return mp_resource;
	}

	graphics::resource* target::get_depth_resource() const
	{
		return mp_depth_resource;
	}

	graphics::descriptor_handle target::get_rtv() const
	{
		return m_rtv_cpu;
	}

	graphics::descriptor_handle target::get_dsv() const
	{
		return m_dsv_cpu;
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

	void target::resize(const target& target)
	{
		const math::vectoru2 target_dim = { target.get_width(), target.get_height() };
		resize(target_dim);
	}

	void target::resize(const math::vectoru2& dimensions)
	{
		if (dimensions != m_current_dimensions)
		{
			// wait for gpu to stop rendering
			renderer_backend::get_instance().wait_gpu_finished();

			mp_resource->release(mp_device);
			mp_device->release(mp_resource);

			// update size:
			m_current_dimensions = dimensions;

			if (m_args.m_has_colour)
			{
				// create new resource
				graphics::tex2D_desc desc{};
				desc.m_arraysize = 1u;
				desc.m_dimensions = { dimensions.x, dimensions.y };
				desc.m_flags = graphics::e_resource_flags::render_target;
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
				desc.m_init_state = graphics::e_resource_state::depth_target;
				mp_depth_resource = mp_device->create_resource(desc);

				recreate_dsv();
			}
		}
	}

	void target::recreate_rtv()
	{
		if (m_rtv_cpu)
		{
			mp_device->create_rtv(m_rtv_cpu, mp_resource);
		}
	}

	void target::recreate_dsv()
	{
		if (m_dsv_cpu)
		{
			mp_device->create_dsv(m_dsv_cpu, mp_depth_resource);
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

