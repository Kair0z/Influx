#include "renderer_pch.h"
#include "influx_renderer/rendergraph/rgpass.h"

namespace influx::renderer
{
	void rgpass_base::set_name(const string& name)
	{
		m_name = name;
	}

	const string& rgpass_base::get_name() const
	{
		return m_name;
	}
}

