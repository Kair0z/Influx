#pragma once
#pragma once
#include "core/basetypes.h"
#include "core/platform/window.h"

namespace influx::graphics
{
	class resource;
	class device;
	class swapchain;
	class descriptor_heap;
	class depth_stencil_view;
}

namespace influx::renderer
{
	struct depth_stencil_create_args final
	{
		depth_stencil_create_args() = default;
		depth_stencil_create_args(uint32 w, uint32 h)
			: m_width{ w }, m_heigth{ h } {}

		uint32 m_width = 1u;
		uint32 m_heigth = 1u;
	};

	// contains a texture resource, as well as a  depth stencil view
	// serving as a depth_stencil for draw commands 
	class depth_stencil
	{
	public:
		graphics::resource* get_resource() const;
		graphics::depth_stencil_view* get_dsv() const;

		uint32 get_width() const;
		uint32 get_height() const;

#if _DEBUG
		void set_name(const string& name);
		const string& get_name() const;
#endif

	private:
		// constructs a depth stencil buffer from create_args, allocating new graphics resources
		explicit depth_stencil(graphics::device* device, graphics::descriptor_heap* dsv_heap, const depth_stencil_create_args& args);

		explicit depth_stencil(graphics::device* device, graphics::resource* resource, graphics::depth_stencil_view* dsv);

		// re-allocates graphics resource, and recreates the dsv
		void resize(const math::vectoru2& new_dimensions);

		// does not allocate a new descriptor handle, but recreates the view
		void recreate_dsv();

		graphics::resource* mp_resource;
		graphics::depth_stencil_view* mp_dsv;
		void* m_descriptor_handle;

		depth_stencil_create_args m_args;
		math::vectoru2 m_current_dimensions;
		graphics::device* mp_device;

#if _DEBUG
		string m_debug_name;
#endif

		// only backend can create depth stencil buffers
		friend class renderer_backend;
	};
}