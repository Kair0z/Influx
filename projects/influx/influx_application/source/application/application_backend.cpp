#include "app_pch.h"
#include "application/application_backend.h"
#include "content/content_manager.h"

// core:
#include "core/log.h"
#include "core/time.h"
#include "core/file.h"
#include "core/scope.h"

// application scene
#include "scene/scene.h"

// platform: win32
#include "core/platform/win32/win32_platform.h"
#include "core/platform/win32/win32_window.h"

// async
#include "influx_async.h"

// input
#include "influx_input.h"

// renderer
#include "influx_renderer.h"

// assets
#include "influx_assets.h"

namespace influx::application
{
	void application::load_render_assets()
	{
		// load meshdata into renderer
		for (const auto& asset : mp_content_manager->get_scenes())
		{
			const assets::scene_data::mesh& mesh = asset.second.m_meshes[0]; // gets the first mesh
			const std::string& name = asset.first;

			renderer::mesh_data mesh_data{};
			for (size_t i = 0u; i < mesh.m_positions.size(); ++i)
			{
				mesh_data.m_vertices.push_back({});
				mesh_data.m_vertices.back().m_position = mesh.m_positions[i];
				// mesh_data.m_vertices.back().m_colour = mesh.m_colours[i];
				mesh_data.m_vertices.back().m_normal = mesh.m_normals[i];
				mesh_data.m_vertices.back().m_texcoords = mesh.m_uvs[i];
			}
			mesh_data.m_indices = mesh.m_indices;

			// load into the renderer
			renderer::load(name, mesh_data);
		}

		// load shaders into renderer
		for (const auto& asset : mp_content_manager->get_shaders())
		{
			const assets::shader_data& shader = asset.second;
			const std::string& name = asset.first;

			renderer::shader_data shader_data{};
			shader_data.m_bytecode = shader.m_compile_result;
			shader_data.m_type = shader.m_type;

			renderer::load(name, shader_data);
		}

		// load textures into renderer
		for (const auto& asset : mp_content_manager->get_images())
		{
			const assets::image_data& image = asset.second;
			const std::string& name = asset.first;

			renderer::texture_data tex_data{};
			tex_data.m_pixels = image.m_pixels;
			tex_data.m_width = image.m_dimensions.x;
			renderer::load(name, tex_data);
		}
	}

	void application::run(const run_args& args)
	{
		process_run_args(args);

		// platform instance handle:
		m_instancehandle = platform::get_current_instance();

		// initialize job system:
		async::init_args async_args{};
		async_args.m_num_workers = 1u;
		async::initialize(async_args);

		if (m_run_args.m_commandlet == false)
		{
			// initialize input
			influx::input::init();

			// create a platform window
			platform::create_window_args window_args{};
			window_args.m_width = args.m_window_width;
			window_args.m_height = args.m_window_height;
			window_args.m_name = args.m_name;
			window_args.m_proc_callback = [](const platform::window_event& ev) { input::push_window_event(ev); };
			m_windowhandle = platform::create_window(window_args);

			// load content
			mp_content_manager = new content_manager(get_resource_directory());
			
			// create renderer
			renderer::init_args render_init_args{};
			render_init_args.m_api_type = renderer::e_render_api::dx12;
			render_init_args.m_resource_dir = get_resource_directory();
			renderer::initialize(render_init_args);

			// load assets into renderer
			load_render_assets();

			// create the scene
			mp_scene = new scene();
			
			// some stack variables
			renderer::present_args present_args{};
			present_args.m_vsync = true;
			time::point initial_tick = time::get_now();
			time::point last_tick = initial_tick;
			frame_time frame_time{};

			// setup targets:
			renderer::target* window_target = renderer::get_window_target(m_windowhandle);
			renderer::target_create_args target_args{};
			target_args.m_has_depth_stencil = true;
			target_args.m_width = window_target->get_width();
			target_args.m_heigth = window_target->get_height();
			renderer::target* scene_target = renderer::create_target(target_args);

			logn("start ticking ...");
			while (!m_is_quit_requested)
			{
				// update frame time
				frame_time.m_delta_seconds = time::get_ms_since<float>(last_tick) * 0.001f;
				frame_time.m_time_seconds += frame_time.m_delta_seconds;
				last_tick = time::get_now();
				
				// returns false if quit event was requested
				if (!platform::poll_window_events(m_windowhandle))
				{
					request_quit();
					continue;
				}
				
				// update scene
				mp_scene->update(frame_time);

				// update the window target
				window_target = renderer::get_window_target(m_windowhandle);

				// draw the render scene
				renderer::draw_scene(mp_scene->get_render_scene(), *scene_target);

				// copy the scene -> window
				renderer::copy_target(*scene_target, *window_target);

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
		m_staged = args.m_staged;

		if (m_staged)
		{
			m_resource_dir = (m_run_args.m_resources_dir.empty()) ?
				platform::get_current_directory() + "/resources/" : m_run_args.m_resources_dir;

			m_asset_dir = (m_run_args.m_assets_dir.empty()) ?
				platform::get_current_directory() + "/assets/" : m_run_args.m_assets_dir;
		}
		else
		{
			// non-staged builds are ran in Influx/bin/[config]/influx_game/ folder
			const string& root_influx = platform::get_current_directory() + "/../../../";
			m_resource_dir = root_influx + "/resources/";
			m_asset_dir = root_influx + "/assets/";
		}
	}

	string application::get_resource_directory() const
	{
		return m_resource_dir;
	}

	string application::get_assets_directory() const
	{
		return m_asset_dir;
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
