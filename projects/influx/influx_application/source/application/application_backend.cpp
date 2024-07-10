#include "app_pch.h"
#include "application/application_backend.h"

// core:
#include "core/log.h"
#include "core/time.h"
#include "core/file.h"

// platform: win32
#include "core/platform/win32/win32_platform.h"
#include "core/platform/win32/win32_window.h"

// input
#include "influx_input.h"

// renderer
#include "influx_renderer.h"

// assets
#include "influx_assets.h"

namespace influx::application
{
	content_cache::content_cache(const string& resource_dir)
	{
		vector<file> fbx_files = get_files_in_directory(resource_dir, true, ".fbx");
		vector<file> png_files = get_files_in_directory(resource_dir, true, ".png");
		vector<file> hlsl_files = get_files_in_directory(resource_dir, true, ".hlsl");

		// load meshes
		for (const file& file : fbx_files)
		{
			assets::scene_load_args args{};
			assets::scene_data& scene_data = m_scenes[file.m_filename];
			assets::load_scene_file(file.m_path_full, scene_data, args);
		}
		
		// load images
		for (const file& file : png_files)
		{
			assets::image_load_args args{};
			assets::image_data& texture_data = m_images[file.m_filename];
			assets::load_image_file(file.m_path_full, texture_data, args);
		}

		// load shaders
		for (const file& file : hlsl_files)
		{
			assets::shader_data& shader_data_vs = m_shaders[file.m_filename + "_vs"];
			assets::shader_data& shader_data_ps = m_shaders[file.m_filename + "_ps"];

			assets::shader_load_args shader_load_args{};
			shader_load_args.m_target = e_shader_target::_6_2;
#if _DEBUG
			shader_load_args.m_compile_debug = true;
#else
			shader_load_args.m_compile_debug = false;
#endif
			shader_load_args.m_pbd = false;
			shader_load_args.m_reflection = false;
			shader_load_args.m_defines = {};

			shader_load_args.m_type = e_shader_type::vs;
			shader_load_args.m_entrypoint = "VSMain";
			influx_assert(assets::load_shader_file(file.m_path_full, shader_data_vs, shader_load_args));

			shader_load_args.m_type = e_shader_type::ps;
			shader_load_args.m_entrypoint = "PSMain";
			influx_assert(assets::load_shader_file(file.m_path_full, shader_data_ps, shader_load_args));
		}
	}

	void application::run(const run_args& args)
	{
		process_run_args(args);

		m_instancehandle = platform::get_current_instance();

		// initialize input:
		input::init();

		if (m_run_args.m_commandlet == false)
		{
			// create a platform window
			platform::create_window_args window_args{};
			window_args.m_width		= args.m_window_width;
			window_args.m_height	= args.m_window_height;
			window_args.m_name		= args.m_name;
			m_windowhandle = platform::create_window(window_args);

			// resources
			mp_content_cache = new content_cache(get_resource_directory());
			
			// create renderer
			renderer::init_args render_init_args{};
			render_init_args.m_api_type = renderer::e_render_api::dx12;
			render_init_args.m_resource_dir = get_resource_directory();
			renderer::initialize(render_init_args);
			
			// create swapchain
			renderer::get_window_target(m_windowhandle);

			renderer::present_args present_args{};
			present_args.m_vsync = true;

			// create scene
			renderer::scene render_scene{};
			{
				math::vectorf3 scene_center = math::vectorf3::zero();

#if 0
				// load meshdata into renderer
				{
					renderer::mesh_data transistor_data{};
					const auto& mesh = transistor_mesh_data.m_meshes[0];
					for (size_t i = 0u; i < mesh.m_positions.size(); ++i)
					{
						transistor_data.m_vertices.push_back({});
						transistor_data.m_vertices.back().m_position = mesh.m_positions[i];
						// transistor_data.m_vertices.back().m_colour = mesh.m_colours[i];
						transistor_data.m_vertices.back().m_normal = mesh.m_normals[i];
						transistor_data.m_vertices.back().m_texcoords = mesh.m_uvs[i];
					}
					transistor_data.m_indices = transistor_mesh_data.m_meshes[0u].m_indices;
					renderer::load("transistor_mesh", transistor_data);
				}
				
				// load shaders into renderer
				{
					renderer::shader_data vs_data{};
					vs_data.m_type = e_shader_type::vs;
					vs_data.m_bytecode = vertex_shader.m_compile_result;

					renderer::shader_data ps_data{};
					ps_data.m_type = e_shader_type::ps;
					ps_data.m_bytecode = pixel_shader.m_compile_result;

					renderer::load("vs_main", vs_data);
					renderer::load("ps_main", ps_data);
				}

				// load textures into renderer
				{
					renderer::texture_data tex_data{};
					tex_data.m_pixels = texture_data.m_pixels;
					tex_data.m_width = texture_data.m_dimensions.x;
					renderer::load("tex_colour", tex_data);
				}
#endif

				// camera
				renderer::camera render_camera{};
				render_camera.m_far_plane = 1000.0f;
				render_camera.m_near_plane = 0.001f;
				render_camera.m_position = { 0.0f, 0.0f, 500.0f };
				render_camera.look_at(scene_center);
				render_scene.m_cameras.push_back(render_camera);

				// meshes
				renderer::mesh_instance mesh_instance{};
				mesh_instance.m_name = "transistor_mesh";
				mesh_instance.m_material_name = "none";
				mesh_instance.m_per_instance_colour;
				mesh_instance.m_transform = math::matrix4x4f::identity();
				render_scene.m_meshes.push_back(mesh_instance);
			}
			
			time::point last_tick = time::get_now();
			float time = 0.0f;
			while (!m_is_quit_requested)
			{
				float delta_seconds = time::get_ms_since<float>(last_tick) * 0.001f;
				time += delta_seconds;
				last_tick = time::get_now();

				render_scene.m_cameras[0].m_position.x = cos(time) * 300.0f;
				render_scene.m_cameras[0].m_position.z = sin(time) * 300.0f;
				render_scene.m_cameras[0].look_at(math::vectorf3::zero());

				// returns false if quit event was requested
				if (!platform::poll_window_events(m_windowhandle))
				{
					request_quit();
					continue;
				}
				else
				{
					// acquire the window target:
					const renderer::target& window_target
						= *renderer::get_window_target(m_windowhandle);

					renderer::draw_scene(render_scene, window_target);

					renderer::present_swapchain(present_args);

					influx::logn("Frame: {}", m_frame++);
				}
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
			m_run_args.m_resources_dir = platform::get_current_directory() + "/resources/";
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
