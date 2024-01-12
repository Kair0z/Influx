#pragma once
#include "influx_renderer.h"

// graphics library include
#include "influx_graphics.h"

namespace influx::renderer
{
	class descriptor_manager;

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

		target* get_window_target(const platform::window_handle& window);

		// issue commands
		void draw_scene(const scene& scene, const target& target);

		void present_swapchain(const present_args& args);

		static descriptor_manager* get_descriptor_manager();

	private:
		graphics::device* mp_device = nullptr;
		graphics::command_queue* mp_graphics_queue = nullptr;
		graphics::swapchain* mp_swapchain = nullptr;
		vector<target*> m_swapchain_targets{};

		graphics::command_list* mp_commandlist = nullptr;
		vector<graphics::command_allocator*> mp_allocators = {};
		graphics::fence* mp_fence = nullptr;

		descriptor_manager* mp_desc_manager = nullptr;

		uint64 m_frame_count = 0u;

		bool m_is_initialized = false;
	};
}
