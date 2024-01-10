
#if 1
#include "core/platform/windows/windows_window.h"
#else
#include "core/platform/null/null_window.h"
#endif

#include "influx_renderer.h"
#include <iostream>

int main()
{
	using namespace influx;

	// create a platform window
	platform::window_handle window = platform::create_window({ { 640u, 480u }, "window" });

	// initialize influx::renderer
	{
		renderer::init_args init{};
		init.m_api_type = renderer::e_render_api::dx12;
		init.m_resource_dir;//
		renderer::initialize(init);
	}

	// create a renderer target from the platform window
	renderer::target window_target = renderer::create_target(window);
	renderer::scene scene = {};
	scene.m_cameras.push_back(renderer::camera{ 90.0f, 0.001f, 1.0f });
	scene.m_meshes.push_back(renderer::mesh_instance{ "mesh", math::matrix4x4f::identity(), "material", math::vectorf4{1,0,0,1} });

	while (true)
	{
		// draw our scene onto the window target
		renderer::draw_scene(scene, window_target);

		// present swapchain
		renderer::present_swapchain({});
	}

	std::cin.get();
	return 0u;
}