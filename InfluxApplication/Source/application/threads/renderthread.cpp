#include "app_pch.h"
#include "renderthread.h"
#include "application/application.h"

#include "Core/Geometry/geometry.h"
#include "Core/Geometry/Sphere.h"
#include "Core/Geometry/quad.h"

#include "influx_renderer.h"
#include "influx_async.h"
#pragma comment(lib, "InfluxRenderer.lib")

#pragma region assimp
#include "foreign/assimp/assimp_helpers.h"
#if _DEBUG
#pragma comment(lib, "assimp-vc142-mtd.lib")
#else
#pragma comment(lib, "assimp-vc142-mt.lib")
#endif
#pragma endregion

#include "imgui/imgui_helpers.h"

namespace influx::application
{
	inline static uint32 g_num_submeshes = 0u;
	inline static vector<renderer::material_data> g_materials{};

	void renderthread::initialize()
	{
		auto app_run_args = application::get_instance().get_run_arguments();
		auto app_resource_directory = application::get_instance().get_resource_directory();

		// initialize renderer
		renderer::init_args args{};
		args.m_api_type = renderer::e_render_api::dx12;
		args.m_resource_dir = app_resource_directory;
		renderer::initialize(args);

		// load meshes, textures & materials into the renderer
		{
			// mesh assets
			{
				assimp_helpers::initialize();

				math::spheref bounding_sphere{};
				assimp_helpers::for_each_mesh_in(app_resource_directory + "/Meshes/Duolingo.fbx",
					[&bounding_sphere](const aiMesh* mesh, const assimp_helpers::add_mesh_info& info)
					{
						renderer::mesh_data result_data{};
						renderer::vertex_data vertex{};

						renderer::material_data material{};
						material.m_albedo = assimp_helpers::parse_material_property<math::vectorf4>(assimp_helpers::e_material_property::diffuse, info.m_material);
						g_materials.push_back(material);

						// vertexbuffer
						for (uint32 i = 0u; i < mesh->mNumVertices; ++i)
						{
							vertex.m_position = assimp_helpers::from_assimp(info.m_world_rotation * mesh->mVertices[i]);
							vertex.m_colour = mesh->HasVertexColors(0u) ? assimp_helpers::from_assimp(mesh->mColors[0u][i]) : material.m_albedo;
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

						++g_num_submeshes;
					});

				assimp_helpers::for_each_texture_in(app_resource_directory + "/Meshes/Duolingo.fbx",
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

			// plane
			renderer::mesh_data plane_mesh_data{};
			using namespace math;
			quadf plane_quad = quadf::up_quad(rectf::square_rect(10.0f), -vectorf3::forward());

			geometry::traverse(plane_quad,
				[&plane_mesh_data](const vectorf3& vertex)
				{
					renderer::vertex_data result_vertex{};
					result_vertex.m_position = vertex;
					plane_mesh_data.m_vertices.push_back(result_vertex);
				},
				[&plane_mesh_data](const uint32 index)
				{
					plane_mesh_data.m_indices.push_back(index);
				});

			renderer::load("plane", plane_mesh_data);
		}

		mp_scene_proxy = new renderer::scene_proxy();
		mp_scene_proxy->m_cameras.push_back({}); // single camera
	}

	inline static void renderthread_imgui_frame(rendersync::game_frame& game_frame)
	{
		// main menu
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("file"))
			{
				if (ImGui::MenuItem("save"))
				{

				}

				if (ImGui::MenuItem("open"))
				{

				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("options"))
			{
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		// camera transform
		if (ImGui::Begin("camera transform"))
		{
			imgui::widget_transform_editor(&game_frame.m_camera_entity.m_transform);

			ImGui::End();
		}

		if (ImGui::Begin("stats"))
		{
			auto render_stats = application::get_average_frame_stats(e_dedicated_thread::renderthread);
			auto game_stats = application::get_average_frame_stats(e_dedicated_thread::gamethread);
			ImGui::SeparatorText("threads");
			ImGui::Text("main: %f ms", 0.0f);
			ImGui::Text("render: %f ms", render_stats.m_ms_total);
			ImGui::Text("game: %f ms", game_stats.m_ms_total);
			ImGui::End();
		}
		// demo window
		//ImGui::ShowDemoWindow();
	}

	void renderthread::tick()
	{
		// sync and recieve a game_frame
		rendersync::game_frame game_frame{};
		sync_to_gamethread(game_frame);

		// build imgui frame
		static function<void(void*)> imgui_frame = [&game_frame](void* _ctx)
		{
			// set the context owned by the renderer-dll
			ImGuiContext* ctx = reinterpret_cast<ImGuiContext*>(_ctx);
			if (ctx == nullptr)
			{
				return;
			}
			ImGui::SetCurrentContext(ctx);
			renderthread_imgui_frame(game_frame);
		};

		// build scene proxy
		build_scene_proxy(game_frame);

		auto app_run_args = application::get_instance().get_run_arguments();
		auto app_window = application::get_instance().get_window_handle();

		// set up render & present arguments
		renderer::render_args render_args{};
		render_args.m_clear_colour = app_run_args.m_window_clear_colour;
		renderer::present_args present_args{};
		present_args.m_vsync = application::is_vsync();

		renderer::render_to_window(
			k_render_scene ? mp_scene_proxy : nullptr, 
			app_window,
			application::is_editor_enabled() ? &imgui_frame : nullptr,
			render_args, 
			present_args);
	}

	void renderthread::build_scene_proxy(const rendersync::game_frame& game_frame)
	{
		// build scene proxy & hardcoded camera
		if (k_render_scene)
		{
			renderer::camera_proxy camera_proxy{};
			mp_scene_proxy->m_cameras[0].m_fov = 90.0f;
			mp_scene_proxy->m_cameras[0].m_near_plane = 0.01f;
			mp_scene_proxy->m_cameras[0].m_far_plane = 1.0f;
			mp_scene_proxy->m_cameras[0].m_position = game_frame.m_camera_entity.m_transform.get_position();
			mp_scene_proxy->m_cameras[0].m_forward = game_frame.m_camera_entity.m_transform.get_forward();

			mp_scene_proxy->m_meshes.resize(game_frame.m_entities.size() * g_num_submeshes);
			if (k_jobify)
			{
				auto handles = async::dispatch_for(game_frame.m_entities.size() * g_num_submeshes,
					[this, &game_frame](uint64 i)
					{
						uint32 entity_idx = static_cast<uint32>(i) / g_num_submeshes;
						uint32 submesh_idx = i % g_num_submeshes;
						const entity& entity = game_frame.m_entities[entity_idx];

						renderer::mesh_proxy mesh{};
						mesh.m_name = "duolingo_mesh_" + std::to_string(submesh_idx);
						mesh.m_transform = entity.m_transform.get_matrix();
						mesh.m_per_instance_colour = g_materials[submesh_idx].m_albedo;

						mp_scene_proxy->m_meshes[(entity_idx * g_num_submeshes) + submesh_idx] = mesh;
					});

				async::wait_for(handles);
			}
			else
			{
				for (uint64 i = 0u; i < game_frame.m_entities.size(); ++i)
				{
					for (uint32 s = 0u; s < g_num_submeshes; ++s)
					{
						renderer::mesh_proxy mesh{};
						mesh.m_name = "duolingo_mesh_" + std::to_string(s);
						mesh.m_transform = game_frame.m_entities[i].m_transform.get_matrix();
						mesh.m_per_instance_colour = g_materials[s].m_albedo;
						mp_scene_proxy->m_meshes[(i * g_num_submeshes) + s] = mesh;
					}
				}
			}
		}
	}
	
	void renderthread::sync_to_gamethread(rendersync::game_frame& game_frame)
	{
		mark_sync_start();
		while (!application::get_render_sync().pop_frame(game_frame))
		{
			// ...
		}
		mark_sync_end();
	}

	void renderthread::cleanup()
	{
		renderer::cleanup();
	}
}