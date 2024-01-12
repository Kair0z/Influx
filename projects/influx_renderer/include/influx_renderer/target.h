#pragma once
#include "core/basetypes.h"
#include "core/platform/window.h"

namespace influx::graphics
{
	class resource;
	class device;
	class swapchain;
	class descriptor_heap;
	class render_target_view;
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
	public:
		graphics::resource* get_resource() const;
		graphics::render_target_view* get_rtv() const;

	private:
		explicit target(graphics::device* device, graphics::descriptor_heap* rtv_heap, const target_create_args& args);

		explicit target(graphics::resource* resource, graphics::render_target_view* rtv);

		static vector<target*> create_swapchain_targets(graphics::device* device, graphics::swapchain* swapchain, graphics::descriptor_heap* rtv_heap);

		friend class renderer_backend;

	private:
		graphics::resource* mp_resource;
		graphics::render_target_view* mp_rtv;
		target_create_args m_args;

		
	};
}