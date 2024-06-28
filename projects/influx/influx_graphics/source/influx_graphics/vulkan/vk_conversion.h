#pragma once
#include "vk_headers.h"
#include "influx_graphics/common.h"

namespace influx::graphics
{
	inline vk::Format convert(e_format format)
	{
		switch (format)
		{
		case e_format::rgba8: return vk::Format::eR8G8B8A8Unorm;
		}

		return vk::Format::eUndefined;
	}
}