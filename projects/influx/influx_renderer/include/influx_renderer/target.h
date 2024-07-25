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
	class depth_stencil_view;
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
		bool m_has_depth_stencil = true;
	};

	// contains a texture resource, as well as a render target view and an optional depth stencil view
	// serving as a target for draw commands
	class target
	{
	public:
		graphics::resource* get_resource() const;
		graphics::render_target_view* get_rtv() const;
		graphics::depth_stencil_view* get_dsv() const;

		INFLUX_RENDER_API uint32 get_width() const;
		INFLUX_RENDER_API uint32 get_height() const;

		INFLUX_RENDER_API bool has_depth_stencil() const;

#if _DEBUG
		void set_name(const string& name);
		const string& get_name() const;
#endif

	private:
		// constructs a target from create_args, allocating new graphics resources
		explicit target(
			graphics::device* device, 
			graphics::descriptor_heap* rtv_heap,
			graphics::descriptor_heap* dsv_heap,
			const target_create_args& args);

		// constructs a target from existing swapchain resources
		explicit target(
			graphics::device* device, 
			graphics::swapchain* swapchain, 
			uint8 swapchain_index, 
			graphics::descriptor_heap* rtv_heap);

		// re-allocates graphics resource, and recreates the rtv
		void resize(const math::vectoru2& new_dimensions);

		// does not allocate a new descriptor handle, but recreates the view
		void recreate_rtv();
		void recreate_dsv();

		graphics::resource* mp_resource;
		graphics::resource* mp_depth_resource;
		graphics::render_target_view* mp_rtv;
		graphics::depth_stencil_view* mp_dsv;
		void* m_rtv_handle;
		void* m_dsv_handle;

		target_create_args m_args;
		math::vectoru2 m_current_dimensions;
		graphics::device* mp_device;

#if _DEBUG
		string m_debug_name;
#endif

		// only backend can create targets
		friend class renderer_backend;
	};
}