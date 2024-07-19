#include "app_pch.h"
#include "application/application_backend.h"

// core:
#include "core/log.h"
#include "core/time.h"
#include "core/file.h"
#include "core/scope.h"

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

	const map<string, assets::scene_data>& content_cache::get_scenes() const
	{
		return m_scenes;
	}

	const map<string, assets::image_data>& content_cache::get_images() const
	{
		return m_images;
	}

	const map<string, assets::shader_data>& content_cache::get_shaders() const
	{
		return m_shaders;
	}

	void application::load_render_assets()
	{
		// load meshdata into renderer
		for (const auto& asset : mp_content_cache->get_scenes())
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
		for (const auto& asset : mp_content_cache->get_shaders())
		{
			const assets::shader_data& shader = asset.second;
			const std::string& name = asset.first;

			renderer::shader_data shader_data{};
			shader_data.m_bytecode = shader.m_compile_result;
			shader_data.m_type = shader.m_type;

			renderer::load(name, shader_data);
		}

		// load textures into renderer
		for (const auto& asset : mp_content_cache->get_images())
		{
			const assets::image_data& image = asset.second;
			const std::string& name = asset.first;

			renderer::texture_data tex_data{};
			tex_data.m_pixels = image.m_pixels;
			tex_data.m_width = image.m_dimensions.x;
			renderer::load(name, tex_data);
		}
	}

	void application::update(renderer::scene& render_scene, const frame_time& time)
	{
		render_scene.m_cameras[0].m_position.x = cos(time.m_time_seconds) * 300.0f;
		render_scene.m_cameras[0].m_position.z = sin(time.m_time_seconds) * 300.0f;
		render_scene.m_cameras[0].look_at(math::vectorf3::zero());
	}

	inline void windows_proc(const platform::window_event& ev)
	{
		// logn("message!");
	}

	void application::run(const run_args& args)
	{
		process_run_args(args);

		m_instancehandle = platform::get_current_instance();

		// initialize input:
		input::init();

		// save assets
		assets::flx_scene scene_file = {};
		scene_file.m_id = 2u;
		scene_file.save(get_assets_directory() + "scene.flx");
		scene_file.save(get_assets_directory() + "scene2.flx");

		if (m_run_args.m_commandlet == false)
		{
			// create a platform window
			platform::create_window_args window_args{};
			window_args.m_width		= args.m_window_width;
			window_args.m_height	= args.m_window_height;
			window_args.m_name		= args.m_name;
			window_args.m_proc_callback = { windows_proc };
			m_windowhandle = platform::create_window(window_args);

			// resources
			logn("loading assets :3 ...");
			time::point time_before_load = time::get_now();
			mp_content_cache = new content_cache(get_resource_directory());
			logn("finished loading assets in {} seconds", time::get_ms_since<float>(time_before_load) * 0.001f);
			
			// create renderer
			renderer::init_args render_init_args{};
			render_init_args.m_api_type = renderer::e_render_api::dx12;
			render_init_args.m_resource_dir = get_resource_directory();
			renderer::initialize(render_init_args);
			
			// create swapchain
			renderer::target* initialTarget = renderer::get_window_target(m_windowhandle);
			// create a depthbuffer
			renderer::depth_stencil_create_args ds_args{};
			ds_args.m_width = initialTarget->get_width();
			ds_args.m_heigth = initialTarget->get_height();
			renderer::depth_stencil* depth_stencil = renderer::create_depth_stencil(ds_args);

			// load assets into renderer
			load_render_assets();

			// create scene
			renderer::scene render_scene{};
			{
				math::vectorf3 scene_center = math::vectorf3::zero();

				// camera
				renderer::camera render_camera{};
				render_camera.m_far_plane = 500.0f; // 1000 makes the depth very imprecise, maybe want to reverse depth
				render_camera.m_near_plane = 0.01f;
				render_camera.m_position = { 0.0f, 0.0f, 400.0f };
				render_camera.look_at(scene_center);
				render_scene.m_cameras.push_back(render_camera);

				// meshes
				renderer::mesh_instance mesh_instance{};
				mesh_instance.m_name = "transistor";
				mesh_instance.m_transform *= math::matrix4x4f::make_scale(math::vectorf3::one() * 2.0f);
				render_scene.m_meshes.push_back(mesh_instance);

				mesh_instance.m_name = "box";
				mesh_instance.m_transform = math::matrix4x4f::make_transform_RH({ 0.0f, 0.0f, 50.0f }, { 0,1,0 });
				mesh_instance.m_transform *= math::matrix4x4f::make_scale(math::vectorf3::one() * 10.0f);
				render_scene.m_meshes.push_back(mesh_instance);
			}
			
			renderer::present_args present_args{};
			present_args.m_vsync = true;

			logn("start ticking ...");
			time::point initial_tick = time::get_now();
			time::point last_tick = initial_tick;
			frame_time frame_time{};
			while (!m_is_quit_requested)
			{
				frame_time.m_delta_seconds = time::get_ms_since<float>(last_tick) * 0.001f;
				frame_time.m_time_seconds = time::get_ms_since<float>(initial_tick) * 0.001f;
				last_tick = time::get_now();

				update(render_scene, frame_time);
				
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

					renderer::draw_scene(render_scene, window_target, *depth_stencil);

					renderer::present_swapchain(present_args);
				}
			}

			delete depth_stencil;
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
