#include "graphics_pch.h"
#include "influx_graphics/common.h"
#include "influx_graphics/resource.h"
#include "influx_graphics/commandlist.h"

namespace influx::graphics
{
	resource::resource(const tex2D_desc& desc)
		: m_type{ e_type::tex2D }
		, m_tex2D_desc{desc}
	{
		m_bytestride = deduce_bytesize(desc.m_format);
		m_bytesize =
			uint64(desc.m_dimensions.x) 
			* uint64(desc.m_dimensions.y)
			* uint64(desc.m_num_mips)
			* uint64(desc.m_arraysize)
			* m_bytestride;

		m_format = desc.m_format;
		m_state = desc.m_init_state;
	}

	resource::resource(const buffer_desc& desc)
		: m_type{ e_type::buffer }
		, m_buffer_desc{desc}
	{
		m_bytesize = m_buffer_desc.m_bytesize;
		m_bytestride = m_buffer_desc.m_bytestride;
		m_format = desc.m_format;
	}

	resource::resource(const tex3D_desc& desc)
		: m_type{ e_type::tex3D }
		, m_tex3D_desc{ desc }
	{
		m_bytestride = deduce_bytesize(desc.m_format);
		m_bytesize =
			uint64(desc.m_dimensions.x)
			* uint64(desc.m_dimensions.y)
			* uint64(desc.m_dimensions.z)
			* uint64(desc.m_num_mips)
			* uint64(desc.m_arraysize)
			* m_bytestride;

		m_format = desc.m_format;
		m_state = desc.m_init_state;
	}

	resource::resource(const cubemap_desc& desc)
		: m_type{ e_type::cubemap }
		, m_cube_desc{ desc }
	{
		m_bytestride = deduce_bytesize(desc.m_format);
		m_bytesize =
			uint64(desc.m_dimensions.x)
			* uint64(desc.m_dimensions.y)
			* uint64(6u)
			* uint64(desc.m_num_mips)
			* uint64(desc.m_arraysize)
			* m_bytestride;

		m_format = desc.m_format;
		m_state = desc.m_init_state;
	}

	resource::resource(const acc_str_desc& desc)
		: m_type{ e_type::acc_struct }
		, m_as_desc{ desc }
	{

	}

	e_format resource::get_format() const
	{
		return m_format;
	}

	uint32 resource::get_width() const
	{
		switch (get_type())
		{
		case resource::e_type::cubemap: return m_cube_desc.m_dimensions.x;
		case resource::e_type::tex2D: return m_tex2D_desc.m_dimensions.x;
		case resource::e_type::tex3D: return m_tex3D_desc.m_dimensions.x;
		}
		return 1u;
	}

	uint32 resource::get_height() const
	{
		switch (get_type())
		{
		case resource::e_type::cubemap: return m_cube_desc.m_dimensions.y;
		case resource::e_type::tex2D: return m_tex2D_desc.m_dimensions.y;
		case resource::e_type::tex3D: return m_tex3D_desc.m_dimensions.y;
		}
		return 1u;
	}

	uint32 resource::get_depth() const
	{
		switch (get_type())
		{
		case resource::e_type::cubemap: return 1u;
		case resource::e_type::tex2D: return 1u;
		case resource::e_type::tex3D: return m_tex3D_desc.m_dimensions.z;
		}
		return 1u;
	}

	uint32 resource::get_arraysize() const
	{
		switch (get_type())
		{
		case resource::e_type::cubemap: return m_cube_desc.m_arraysize;
		case resource::e_type::tex2D: return m_tex2D_desc.m_arraysize;
		case resource::e_type::tex3D: return m_tex3D_desc.m_arraysize;
		}
		return 1u;
	}

	uint32 resource::get_num_subresources() const
	{
		switch (get_type())
		{
		case resource::e_type::cubemap: return m_cube_desc.m_arraysize;
		case resource::e_type::tex2D: return m_tex2D_desc.m_arraysize;
		case resource::e_type::tex3D: return m_tex3D_desc.m_arraysize;
		}
		return 1u;
	}

	uint64 resource::get_bytesize() const
	{
		return m_bytesize;
	}

	uint64 resource::get_bytestride() const
	{
		return m_bytestride;
	}

	uint32 resource::get_num_elements() const
	{
		return (uint32)(get_bytesize() / get_bytestride());
	}

	e_resource_state resource::get_state() const
	{
		return m_state;
	}

	e_resource_state resource::get_previous_state() const
	{
		return m_previous_state;
	}

	range<uint64> resource::get_full_range() const
	{
		return range<uint64>(0u, m_bytesize);
	}

	result<> resource::transition(commandlist* cmdlist, e_resource_state new_state)
	{
		result<> res = {};
		if (m_state == new_state)
		{
			return result<>::make_error("warning: skipping transition to already current state.");
		}

		res = cmdlist->transition_resource(this, m_state, new_state);

		m_previous_state = m_state;
		m_state = new_state;
		return res;
	}

	result<> resource::revert_transition(commandlist* cmdlist)
	{
		result<> res = {};
		influx_assert(m_previous_state != m_state);

		res = cmdlist->transition_resource(this, m_state, m_previous_state);

		e_resource_state state_before = m_state;
		m_state = m_previous_state;
		m_previous_state = state_before;
		return res;
	}

	resource::e_type resource::get_type() const
	{
		return m_type;
	}
}