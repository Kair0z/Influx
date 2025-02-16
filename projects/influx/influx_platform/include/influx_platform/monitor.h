#pragma once
#include "influx_platform/platform.h"

// influx::core
#include "core/container/vector.h"
#include "core/math/vector.h"
namespace influx::platform
{
	class monitor
	{
	public:
		INFLUX_PLATFORM_API 
		static vector<monitor> query_monitors();

		math::vectorf2 m_mainpos;
		math::vectoru2 m_mainsize;
		math::vectorf2 m_workpos;
		math::vectoru2 m_worksize;
		float m_dpi_scale;
		bool m_is_primary = false;
		void* m_platform_handle;
	};
}