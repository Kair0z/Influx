#include "rendergraph_pch.h"

#include "rendergraph.h"
#include "rgpass.h"

#include "influx_graphics/renderpass.h"

namespace influx::rendergraph
{
	rgpass::rgpass(e_rgpass_type type, e_rgpass_flags flags)
	{
	}

	bool rgpass::is_culled() const
	{
		return false;
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

