#include "renderer_pch.h"

#include "influx_renderer/rendergraph/rendergraph.h"
#include "influx_renderer/rendergraph/rgpass.h"

#include "influx_graphics/renderpass.h"

namespace influx::renderer
{
	rgpass_base::rgpass_base(e_rgpass_type type, e_rgpass_flags flags)
	{
	}

	bool rgpass_base::is_culled() const
	{
		return false;
	}

	bool rgpass_base::allow_uav_writes() const
	{
		return false;
	}

	void rgpass_base::set_id(rgpass_id id)
	{
		m_id = id;
	}

	e_rgpass_type rgpass_base::get_type() const
	{
		return m_type;
	}

	bool rgpass_base::has_dependency(rgpass_base* a, rgpass_base* b)
	{
		return false;
	}

	uint32 rgpass_base::get_width() const
	{
		return m_width;
	}

	uint32 rgpass_base::get_height() const
	{
		return m_height;
	}
}

