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

#if _DEBUG
		void set_name(const string& name);
		const string& get_name() const;
#endif

	private:
		// constructs a target from create_args, allocating new graphics resources
		explicit target(graphics::device* device, graphics::descriptor_heap* rtv_heap, const target_create_args& args);

		// constructs a target from existing swapchain resources
		explicit target(graphics::device* device, graphics::swapchain* swapchain, uint8 swapchain_index, graphics::descriptor_heap* rtv_heap);

		explicit target(graphics::device* device, graphics::resource* resource, graphics::render_target_view* rtv);

		// re-allocates graphics resource, and recreates the rtv
		void resize(const math::vectoru2& new_dimensions);

		// does not allocate a new descriptor handle, but recreates the view
		void recreate_rtv();

		graphics::resource* mp_resource;
		graphics::render_target_view* mp_rtv;
		void* m_descriptor_handle;

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