#include "app_pch.h"
#include "application/application_backend.h"

// platform: win32
#include "core/platform/win32/win32_platform.h"
#include "core/platform/win32/win32_window.h"

// renderer
#include "influx_renderer.h"

namespace influx::application
{
	void application::run(const run_args& args)
	{
		process_run_args(args);

		m_instancehandle = platform::get_current_instance();

		if (m_run_args.m_commandlet == false)
		{
			// create a platform window
			platform::create_window_args window_args{};
			window_args.m_width		= args.m_window_width;
			window_args.m_height	= args.m_window_height;
			window_args.m_name		= args.m_name;
			m_windowhandle = platform::create_window(window_args);

			// create renderer
			renderer::init_args render_init_args{};
			render_init_args.m_api_type = renderer::e_render_api::dx12;
			render_init_args.m_resource_dir = get_resource_directory();
			renderer::initialize(render_init_args);

			renderer::target_create_args target_args{};
			target_args.m_width = args.m_window_width;
			target_args.m_heigth = args.m_window_height;
			renderer::target* window_target = renderer::create_target(target_args);

			renderer::present_args present_args{};
			present_args.m_vsync = true;

			// create scene
			renderer::scene render_scene{};
			{
				math::vectorf3 scene_center = math::vectorf3::zero();

				// data loading
				renderer::mesh_data scene_data{};
				scene_data.m_indices;
				scene_data.m_vertices;
				renderer::load("scene_mesh", scene_data);

				// camera
				renderer::camera render_camera{};
				render_camera.m_far_plane = 1000.0f;
				render_camera.m_near_plane = 0.001f;
				render_camera.m_position = { 0.0f, 0.0f, 10.0f };
				render_camera.look_at(scene_center);
				render_scene.m_cameras.push_back(render_camera);

				// meshes
				renderer::mesh_instance mesh_instance{};
				mesh_instance.m_name = "scene_mesh";
				mesh_instance.m_material_name = "none";
				mesh_instance.m_per_instance_colour;
				mesh_instance.m_transform = math::matrix4x4f::identity();
				render_scene.m_meshes.push_back(mesh_instance);
			}
			
			while (true)
			{
				renderer::draw_scene(render_scene, *window_target);
				renderer::present_swapchain(present_args);
			}
		}
	}

	void application::request_quit()
	{
		m_is_quit_requested = true;
	}

	bool application::is_quit_requested()
	{
		return get_instance().m_is_quit_requested;
	}

	bool application::is_vsync()
	{
		return get_instance().m_run_args.m_vsync || k_force_vsync;
	}

	bool application::is_editor_enabled()
	{
		return get_instance().m_run_args.m_enable_editor && !is_commandlet();
	}

	bool application::is_game_enabled()
	{
		return get_instance().m_run_args.m_enable_game && !is_commandlet();
	}

	bool application::is_scene_render_enabled()
	{
		return true && k_allow_scenerender;
	}

	bool application::is_commandlet()
	{
		return get_instance().m_run_args.m_commandlet;
	}

	void application::process_run_args(const run_args& args)
	{
		m_run_args = args;

		if (m_run_args.m_resources_dir.empty())
		{
			m_run_args.m_resources_dir = platform::get_current_directory() + "/Resources/";
		}
	}

	string application::get_resource_directory() const
	{
		return m_run_args.m_resources_dir;
	}

	run_args application::get_run_arguments() const
	{
		return m_run_args;
	}

	platform::window_handle application::get_window_handle() const
	{
		return m_windowhandle;
	}

	platform::instance_handle application::get_instance_handle() const
	{
		return m_instancehandle;
	}

#pragma region frontend api
	void run(const run_args& args)
	{
		application::get_instance().run(args);
	}

	void quit()
	{
		application::get_instance().request_quit();
	}
#pragma endregion
}
