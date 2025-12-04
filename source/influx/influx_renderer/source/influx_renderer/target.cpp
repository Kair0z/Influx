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
	static graphics::tex2D_desc get_default_color_desc(const math::vectoru2& dimensions)
	{
		graphics::tex2D_desc desc{};
		desc.m_arraysize = 1u;
		desc.m_dimensions = dimensions;
		desc.m_format = graphics::e_format::rgba8;
		desc.m_num_mips = 1u;
		desc.m_sample_count = 1u;
		desc.m_bindflags = graphics::e_bind_flags::rtv | graphics::e_bind_flags::uav;
		desc.m_init_state = graphics::e_resource_state::render_target;
		desc.m_allow_uav = true;
		return desc;
	};

	static graphics::tex2D_desc get_default_depth_desc(const math::vectoru2& dimensions)
	{
		graphics::tex2D_desc desc{};
		desc.m_arraysize = 1u;
		desc.m_dimensions = dimensions;
		desc.m_format = graphics::e_format::d32;
		desc.m_num_mips = 1u;
		desc.m_sample_count = 1u;
		desc.m_bindflags = graphics::e_bind_flags::dsv;
		desc.m_init_state = graphics::e_resource_state::depth_target;
		return desc;
	};

	// constructs a target from create_args, allocating new graphics resources
	target::target(graphics::device* device, const target_create_args& args)
		: mp_device{device}
		, m_createargs{args}
	{
		influx_assert(args.m_has_colour || args.m_has_depth_stencil);

		descriptor_manager& desc_manager = *renderer_backend::get_descriptor_manager();
		if (args.m_has_colour)
		{
			// create the colour resource
			graphics::tex2D_desc desc = get_default_color_desc({ args.m_width, args.m_heigth });
			mp_resource = device->create_resource(desc);

			m_rtv_cpu = desc_manager.create_rtv(*device, *mp_resource);
			m_srv_cpu = desc_manager.create_srv(*device, *mp_resource);
		}
		if (args.m_has_depth_stencil)
		{
			graphics::tex2D_desc desc = get_default_depth_desc({ args.m_width, args.m_heigth });
			mp_depth_resource = device->create_resource(desc);
			m_dsv_cpu = desc_manager.create_dsv(*device, *mp_depth_resource);
		}

		set_name(get_name());

		m_current_dimensions = { args.m_width, args.m_heigth };
	}

	target::~target()
	{
		descriptor_manager* desc_man = renderer_backend::get_descriptor_manager();
		if (desc_man && m_rtv_cpu)
		{
			desc_man->cleanup_rtv(m_rtv_cpu);
			m_rtv_cpu = nullptr;
		}
		if (desc_man && m_srv_cpu)
		{
			desc_man->cleanup_srv(m_srv_cpu);
			m_srv_cpu = nullptr;
		}
		if (desc_man && m_dsv_cpu)
		{
			desc_man->cleanup_dsv(m_dsv_cpu);
			m_dsv_cpu = nullptr;
		}

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

	graphics::descriptor_handle target::get_srv() const
	{
		return m_srv_cpu;
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
		return !m_createargs.m_has_colour && m_createargs.m_has_depth_stencil;
	}

	void target::resize(const target& target)
	{
		const math::vectoru2 target_dim = { target.get_width(), target.get_height() };
		resize(target_dim);
	}

	void target::resize(const math::vectoru2& dimensions)
	{
		m_prev_dimensions = m_current_dimensions;
		if (dimensions != m_current_dimensions)
		{
			// wait for gpu to stop rendering
			renderer_backend::get_instance().wait_until_gpu_idle();
			mp_device->release(mp_resource);
			mp_device->release(mp_depth_resource);

			if (m_createargs.m_has_colour)
			{
				// create new resource
				graphics::tex2D_desc desc = get_default_color_desc(dimensions);
				mp_resource = mp_device->create_resource(desc);
				recreate_rtv();
				recreate_srv();
			}

			if (m_createargs.m_has_depth_stencil)
			{
				graphics::tex2D_desc desc = get_default_depth_desc(dimensions);
				mp_depth_resource = mp_device->create_resource(desc);
				recreate_dsv();
			}

			// re-apply name to resources
			set_name(get_name());

			m_current_dimensions = dimensions;
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

	void target::recreate_srv()
	{
		if (m_srv_cpu)
		{
			mp_device->create_texture_srv(m_srv_cpu, mp_resource);
		}
	}

	void target::set_name(const debug_name& name)
	{
		if (mp_resource && mp_resource->is_valid())
		{
			mp_resource->set_name(name);
		}

		if (mp_depth_resource && mp_depth_resource->is_valid())
		{
			mp_depth_resource->set_name(get_name_depth(name));
		}

		m_depth_name = get_name_depth(m_createargs.m_name);
		m_createargs.m_name = name;
	}

	const debug_name& target::get_name() const
	{
		return m_createargs.m_name;
	}
	const debug_name& target::get_name_depth() const
	{
		return m_depth_name;
	}
	debug_name target::get_name_depth(const debug_name& base)
	{
		return string(string(base) + k_depth_name_postfix);
	}
}

