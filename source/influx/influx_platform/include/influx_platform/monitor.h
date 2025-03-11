#pragma once
#include "influx_platform/platform.h"

// influx::core
#include "core/container/vector.h"
#include "core/math/vector.h"
#include "core/geometry/rect.h"

namespace influx::platform
{
	class window;
	class monitor
	{
	public:
		enum class e_space : uint8
		{
			work,
			full,
			count
		};

	public:
		INFLUX_PLATFORM_API 
		static vector<monitor> query_monitors();

		INFLUX_PLATFORM_API
		static monitor from_window(const window& window);

		INFLUX_PLATFORM_API
		math::rectu get_rect(e_space space = e_space::work) const;

		math::vectorf2 m_mainpos;
		math::vectoru2 m_mainsize;
		math::vectorf2 m_workpos;
		math::vectoru2 m_worksize;
		float m_dpi_scale;
		bool m_is_primary = false;
		void* m_platform_handle;
	};
}