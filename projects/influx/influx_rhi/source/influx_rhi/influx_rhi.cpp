#include "rhi_pch.h"
#include "influx_rhi.h"

namespace influx::rhi
{
	device* device::create(e_api_type api)
	{
		return new device(api);
	}
}