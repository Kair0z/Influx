#include "app_pch.h"

#include "application/application.h"
#include "influx_renderer.h"
#include "influx_async.h"

#if INFLUX_APP_USES_WINDOWS
#include "core/platform/windows_platform.h"
#endif

#include "Core/Math/Random.h"
#include "Core/Time.h"

#pragma comment(lib, "InfluxRenderer.lib")
#pragma comment(lib, "InfluxAsync.lib")

// We're using Assimp libary for loading .FBX files...
#include "foreign/assimp/assimp_helpers.h"
#if _DEBUG
#pragma comment(lib, "assimp-vc142-mtd.lib")
#else
#pragma comment(lib, "assimp-vc142-mt.lib")
#endif

#include <iostream>

namespace influx::application
{
	#if INFLUX_APP_USES_WINDOWS
	inline static ::LRESULT windows_procedure(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_DESTROY:
		{
			application::get_instance().request_quit();
			return 0;
		}

		default:
			return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	#endif

	void application::run(const run_args& args)
	{
		m_run_args = args;
		if (m_run_args.m_resources_dir.empty())
		{
			m_run_args.m_resources_dir = platform::get_current_directory() + "/Resources/";
		}

		// initialize
		{
			m_instancehandle = platform::get_current_instance();
			
			if (!args.m_commandlet)
			{
				// create a window
				platform::create_window_args window_args{};
				window_args.m_width = (int)args.m_window_width;
				window_args.m_height = (int)args.m_window_height;
				window_args.m_name = args.m_name;
				m_windowhandle = platform::create_window(window_args, true, windows_procedure);

				m_renderthread = std::thread(&application::run_renderthread, this);
				m_gamethread = std::thread(&application::run_gamethread, this);

				// run main thread
				run_mainthread();

				// cleanup
				if (m_renderthread.joinable()) m_renderthread.join();
				if (m_mainthread.joinable()) m_mainthread.join();
			}
		}
	}

	void application::request_quit()
	{
		m_is_quit_requested = true;
	}

	void application::run_mainthread()
	{
		uint64 mainthread_frame = 0u;
		vector<platform::e_windowevent> out_events{};
		while (!m_is_quit_requested)
		{
			// window events
			if (!platform::poll_window_events(out_events, m_windowhandle))
			{
				request_quit();
				break;
			}
			
			// handle events
			for (platform::e_windowevent e : out_events)
			{
			}

			// log stats
			if (mainthread_frame % (512 * 512) == 0u && mainthread_frame != 0u)
			{
				mainthread_log();
			}

			++mainthread_frame;
		}

		int a = 0; 
		a++;
	}

	void application::run_gamethread()
	{
		frame_stats this_frame_stat{};
		float seconds_synced = 0.0f;
		time::point frame_start = time::get_now();
		const uint64 frame_diff = m_run_args.m_max_thread_frame_difference;

		// start
		// temp: create entities
		constexpr uint64 k_num_entities = 14u;
		m_entities.reserve(k_num_entities);
		for (uint64 i = 0u; i < k_num_entities; ++i)
		{
			m_entities.push_back(i);
		}

		while (!m_is_quit_requested)
		{
			frame_start = time::get_now();

			if (m_gamethread_frame > frame_diff)
			{
				wait_for_renderthread_reaching(m_gamethread_frame - frame_diff, wait_args{ &seconds_synced });
			}
			
			for (entity& entity : m_entities)
			{
				// update
				entity.m_id++;
				entity.m_transform = math::transform3D(
					random::get_random_unit_vectorf3() * 5.0f,
					math::quaternion::identity(),
					math::vectorf3::one());
			}

			m_camera_entity.m_transform.set_position({ 0.0f, 0.0f, 10.0f });
			m_camera_entity.m_transform.set_forward({ 0.0f, 0.0f, -1.0f });

			this_frame_stat.m_ms_total = math::maximum(math::k_epsilon, time::get_ms_between<float>(time::get_now(), frame_start));
			this_frame_stat.m_pc_sync = math::is_zero(this_frame_stat.m_ms_total) ? 0.0f : (seconds_synced * 1000.0f) / this_frame_stat.m_ms_total;
			m_gamethread_state.m_stats.pop_to_push(this_frame_stat);
			++m_gamethread_frame;
		}
	}

	void application::run_renderthread()
	{
		renderer::init_args args{};
		args.m_api_type = renderer::e_render_api::dx12;
		args.m_resource_dir = m_run_args.m_resources_dir;
		renderer::initialize(args);

		// load assets into the renderer
		uint32 num_submeshes{}; 
		vector<renderer::material_data> materials{};
		renderthread_loadassets(num_submeshes, materials);

		renderer::render_args render_args{};
		renderer::present_args present_args{};
		present_args.m_vsync = m_run_args.m_vsync;

		renderer::scene_proxy scene_proxy{};
		renderer::camera_proxy camera_proxy{};
		scene_proxy.m_cameras.push_back(camera_proxy);
		scene_proxy.m_cameras[0].m_fov = 90.0f;
		scene_proxy.m_cameras[0].m_near_plane = 0.01f;
		scene_proxy.m_cameras[0].m_far_plane = 1.0f;
		scene_proxy.m_cameras[0].m_position = m_camera_entity.m_transform.get_position();
		scene_proxy.m_cameras[0].m_forward = m_camera_entity.m_transform.get_forward();

		frame_stats this_frame_stats{};
		float seconds_synced = 0.0f;
		time::point frame_start = time::get_now();
		while (!m_is_quit_requested)
		{
			frame_start = time::get_now();

			// make sure this frame's passed simulation
			wait_for_gamethread_reaching(m_renderthread_frame + 1u, wait_args{ &seconds_synced });

			// update render proxies
			scene_proxy.m_meshes.resize(m_entities.size() * num_submeshes);
			for (uint64 i = 0u; i < m_entities.size(); ++i)
			{
				for (uint32 s = 0u; s < num_submeshes; ++s)
				{
					renderer::mesh_proxy mesh{};
					mesh.m_name = "duolingo_mesh_" + std::to_string(s);
					mesh.m_transform = m_entities[i].m_transform.get_matrix();
					mesh.m_per_instance_colour = materials[s].m_albedo;
					scene_proxy.m_meshes[(i * num_submeshes) + s] = mesh;
				}
			}
			
			// render
			renderer::render_to_window(&scene_proxy, render_args, m_windowhandle, present_args);

			this_frame_stats.m_ms_total = math::maximum(math::k_epsilon, time::get_ms_between<float>(time::get_now(), frame_start));
			this_frame_stats.m_pc_sync = math::is_zero(this_frame_stats.m_ms_total) ? 0.0f : (seconds_synced * 1000.0f) / this_frame_stats.m_ms_total;
			m_renderthread_state.m_stats.pop_to_push(this_frame_stats);
			++m_renderthread_frame;
		}

		renderer::cleanup();
	}

	void application::renderthread_loadassets(uint32& num_submeshes, vector<renderer::material_data>& materials)
	{
		assimp_helpers::initialize();

		math::spheref bounding_sphere{};
		assimp_helpers::for_each_mesh_in(m_run_args.m_resources_dir + "/Meshes/Duolingo.fbx",
			[&num_submeshes, &materials, &bounding_sphere](const aiMesh* mesh, const assimp_helpers::add_mesh_info& info)
			{
				renderer::mesh_data result_data{};
				renderer::vertex_data vertex{};

				renderer::material_data material{};
				material.m_albedo = assimp_helpers::parse_material_property<math::vectorf4>(assimp_helpers::e_material_property::diffuse, info.m_material);
				materials.push_back(material);

				// vertexbuffer
				for (uint32 i = 0u; i < mesh->mNumVertices; ++i)
				{
					vertex.m_position = assimp_helpers::from_assimp(info.m_world_rotation * mesh->mVertices[i]);
					vertex.m_colour = mesh->HasVertexColors(0u) ? assimp_helpers::from_assimp(mesh->mColors[0u][i]) : math::vectorf4{};
					vertex.m_normal = mesh->HasNormals() ? assimp_helpers::from_assimp(mesh->mNormals[i]) : math::vectorf3{};
					vertex.m_texcoords = mesh->HasTextureCoords(0u) ? assimp_helpers::from_assimp(mesh->mTextureCoords[0u][i]).get_xy() : math::vectorf2{};
					result_data.m_vertices.push_back(vertex);
				}

				// indexbuffer
				for (uint32 f = 0u; f < mesh->mNumFaces; ++f)
				{
					for (uint32 i = 0u; i < mesh->mFaces[f].mNumIndices; ++i)
					{
						result_data.m_indices.push_back(mesh->mFaces[f].mIndices[i]);
					}
				}

				// load into the renderer
				renderer::load("duolingo_mesh_" + to_string(info.m_idx), result_data);

				++num_submeshes;
			});

		assimp_helpers::for_each_mesh_in(m_run_args.m_resources_dir + "/Meshes/Duolingo.fbx",
			[&num_submeshes, &materials](const aiMesh* mesh, const assimp_helpers::add_mesh_info& info)
			{
				renderer::mesh_data result_data{};
				renderer::vertex_data vertex{};

				// vertexbuffer
				for (uint32 i = 0u; i < mesh->mNumVertices; ++i)
				{
					vertex.m_position = assimp_helpers::from_assimp(info.m_world_rotation * mesh->mVertices[i]);
					vertex.m_colour = mesh->HasVertexColors(0u) ? assimp_helpers::from_assimp(mesh->mColors[0u][i]) : math::vectorf4{};
					vertex.m_normal = mesh->HasNormals() ? assimp_helpers::from_assimp(mesh->mNormals[i]) : math::vectorf3{};
					vertex.m_texcoords = mesh->HasTextureCoords(0u) ? assimp_helpers::from_assimp(mesh->mTextureCoords[0u][i]).get_xy() : math::vectorf2{};
					result_data.m_vertices.push_back(vertex);
				}

				// indexbuffer
				for (uint32 f = 0u; f < mesh->mNumFaces; ++f)
				{
					for (uint32 i = 0u; i < mesh->mFaces[f].mNumIndices; ++i)
					{
						result_data.m_indices.push_back(mesh->mFaces[f].mIndices[i]);
					}
				}

				// load into the renderer
				renderer::load("duolingo_mesh_" + to_string(info.m_idx), result_data);

				++num_submeshes;
			});

		assimp_helpers::for_each_texture_in(m_run_args.m_resources_dir + "/Meshes/Duolingo.fbx",
			[](const aiTexture* texture, uint32 index)
			{
				const uint32 num_pixels = (texture->mWidth * texture->mHeight);

				renderer::texture_data result_data{};
				result_data.m_width = texture->mWidth;
				result_data.m_pixels.reserve(num_pixels);
				for (uint32 i = 0u; i < num_pixels; ++i)
				{
					const aiTexel& texel = texture->pcData[i];
					result_data.m_pixels.push_back(assimp_helpers::from_assimp(texel.operator aiColor4D()));
				}

				renderer::load("duolingo_texture_" + to_string(index), result_data);
			});

		assimp_helpers::cleanup();
	}

	void application::wait_for_renderthread_reaching(const uint64 frame_to_reach, const wait_args& args)
	{
		time::point before_wait = time::get_now();
		while (m_renderthread_frame < frame_to_reach)
		{
			// wait...
		}

		if (args.mp_out_seconds_waited != nullptr)
		{
			(*args.mp_out_seconds_waited) = time::get_ms_between<float>(time::get_now(), before_wait) * 0.001f;
		}
	}

	void application::wait_for_gamethread_reaching(const uint64 frame_to_reach, const wait_args& args)
	{
		time::point before_wait = time::get_now();
		while (m_gamethread_frame < frame_to_reach)
		{
			// wait...
		}

		if (args.mp_out_seconds_waited != nullptr)
		{
			(*args.mp_out_seconds_waited) = time::get_ms_between<float>(time::get_now(), before_wait) * 0.001f;
		}
	}

	void application::mainthread_log()
	{
		system("cls");
		frame_stats game_stats = m_gamethread_state.m_stats.get_average_value(64u);
		frame_stats render_stats = m_renderthread_state.m_stats.get_average_value(64u);

		std::cout << "[Game]  \tFPS: " << 1.0f / (game_stats.m_ms_total * 0.001f) << "\t| ms: " << game_stats.m_ms_total << "\t| " << "Sync: " << 100.0f * game_stats.m_pc_sync << "%\n";
		std::cout << "[Render]\tFPS: " << 1.0f / (render_stats.m_ms_total * 0.001f) << "\t| ms: " << render_stats.m_ms_total << "\t| " << "Sync: " << 100.0f * render_stats.m_pc_sync << "%\n";
	}

#pragma region apifunctions
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
