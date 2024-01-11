#pragma once
#include "core/basetypes.h"
#include "core/platform/window.h"

namespace influx::graphics
{
	class resource;
	class device;
}

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

	// contains a texture resource, as well as a render target view
	// serving as a target for draw commands
	class target
	{
	private:
		explicit target(graphics::device* device, const platform::window_handle& from_window);
		explicit target(graphics::device* device, const target_create_args& args);
		friend class renderer_backend;

	private:
		graphics::resource* mp_resource;
		static target_create_args make_from_window(const platform::window_handle& window);
	};
}