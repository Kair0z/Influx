#pragma once
#include "influx_renderer.h"

// graphics library include
#include "influx_graphics.h"

namespace influx::renderer
{
	// backend singleton keeping static state for the renderer
	class renderer_backend final
		: public singleton<renderer_backend>
	{
	public:
		void initialize(const init_args& args);
		bool is_initialized() const;
		void cleanup();

		// create a target to render to
		target* create_target(const target_create_args& args);

		// implicitly creates a swapchain
		target* create_target(const platform::window_handle& window);

		// issue commands
		void draw_scene(const scene& scene, const target& target);

		void present_swapchain(const present_args& args);

	private:
		graphics::device* mp_device;
		graphics::command_queue* mp_graphics_queue;
		graphics::swapchain* mp_swapchain;

		graphics::command_list* mp_commandlist;
		vector<graphics::command_allocator*> mp_allocators;

		graphics::fence* mp_fence;

		bool m_is_initialized;
	};
}
