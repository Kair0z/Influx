#include "rendergraph_pch.h"

#include "rendergraph.h"
#include "rgpass.h"

#include "core/enum.h"
#include "rgcommon.h"

#include "influx_graphics/renderpass.h"

namespace influx::rendergraph
{
	uint32 rgpass::get_num_reads() const
	{
		return static_cast<uint32>(m_buffer_reads.size() + m_texture_reads.size());
	}

	uint32 rgpass::get_num_writes() const
	{
		return static_cast<uint32>(m_buffer_writes.size() + m_texture_writes.size());
	}

	rgpass::rgpass(e_rgpass_type type, e_rgpass_flags flags)
	{
	}

	bool rgpass::is_culled() const
	{
		return false;
	}

	bool rgpass::can_be_culled() const
	{
		return !has_any_flag(m_flags, e_rgpass_flags::force_no_cull);
	}

	bool rgpass::allow_uav_writes() const
	{
		return false;
	}

	void rgpass::set_id(rgpass_id id)
	{
		m_id = id;
	}

	e_rgpass_type rgpass::get_type() const
	{
		return m_type;
	}

	bool rgpass::has_dependency(rgpass* a, rgpass* b)
	{
		return false;
	}

	uint32 rgpass::get_width() const
	{
		return m_width;
	}

	uint32 rgpass::get_height() const
	{
		return m_height;
	}
}

