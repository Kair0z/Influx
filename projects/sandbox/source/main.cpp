
#include <iostream>
#include <thread>

#include "influx_application.h"

#if 0
void foo()
{
	// create a platform window
	platform::window_handle window = platform::create_window({ { 640u, 480u }, "window" });

	// initialize influx::renderer
	{
		renderer::init_args init{};
		init.m_api_type = renderer::e_render_api::dx12;
		init.m_resource_dir;//
		renderer::initialize(init);
	}

	renderer::scene scene = {};
	scene.m_cameras.push_back(renderer::camera{ 90.0f, 0.001f, 1.0f });
	scene.m_meshes.push_back(renderer::mesh_instance{ "mesh", math::matrix4x4f::identity(), "material", math::vectorf4{1,0,0,1} });

	while (true)
	{
		const renderer::target& window_target = *renderer::get_window_target(window);

		// acquire a swapchain frame to render
		renderer::acquire_swapchain_frame();

		// draw our scene onto the window target
		renderer::draw_scene(scene, window_target);

		// present our window
		renderer::present_swapchain({});
	}
}
#endif

int main()
{
	using namespace influx;
	application::run_args args{};
	args.m_window_width = 640u;
	args.m_window_height = 480u;

	std::thread app_thread;
	application::run_async(app_thread, args);

	if (app_thread.joinable())
	{
		app_thread.join();
	}

	return 0u;
}