#include "graphics_pch.h"
#include "influx_graphics/common.h"
#include "influx_graphics/resource.h"
#include "influx_graphics/commandlist.h"

namespace influx::graphics
{
	resource::resource(const tex2D_desc& desc)
		: m_tex2D_desc{desc}
	{
		m_bytesize =
			size_t(desc.m_dimensions.x) 
			* size_t(desc.m_dimensions.y)
			* size_t(desc.m_num_mips)
			* size_t(desc.m_arraysize)
			* deduce_bytesize(desc.m_format);

		m_format = desc.m_format;
		m_state = desc.m_init_state;
	}

	resource::resource(const buffer_desc& desc)
		: m_buffer_desc{desc}
	{
		m_bytesize = m_buffer_desc.m_bytesize;
		m_bytestride = m_buffer_desc.m_bytestride;
		m_format = desc.m_format;
	}

	e_format resource::get_format() const
	{
		return m_format;
	}

	uint32 resource::get_width() const
	{
		return m_tex2D_desc.m_dimensions.x;
	}

	uint32 resource::get_height() const
	{
		return m_tex2D_desc.m_dimensions.y;
	}

	size_t resource::get_bytesize() const
	{
		return m_bytesize;
	}

	size_t resource::get_bytestride() const
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

	void resource::transition(command_list* cmdlist, e_resource_state new_state)
	{
		if (m_state == new_state)
		{
			return;
		}

		cmdlist->transition_resource(this, m_state, new_state);

		m_previous_state = m_state;
		m_state = new_state;
	}

	void resource::revert_transition(command_list* cmdlist)
	{
		influx_assert(m_previous_state != m_state);

		cmdlist->transition_resource(this, m_state, m_previous_state);

		e_resource_state state_before = m_state;
		m_state = m_previous_state;
		m_previous_state = state_before;
	}
}