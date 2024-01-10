#pragma once
#include "core/basetypes.h"
#include "core/platform/window.h"

namespace influx::renderer
{
	struct target_create_args final
	{
		target_create_args() = default;
		target_create_args(uint32 w, uint32 h)
			: m_width{ w }, m_heigth{ h } {}

		uint32 m_width = 1u;
		uint32 m_heigth = 1u;
	};

	class target
	{
	public:
		explicit target(const platform::window_handle& from_window);
		explicit target(const target_create_args& args);
	};
}