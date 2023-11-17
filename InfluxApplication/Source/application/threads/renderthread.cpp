#include "app_pch.h"
#include "renderthread.h"
#include "application/application.h"

#include "Core/Geometry/geometry.h"
#include "Core/Geometry/Sphere.h"
#include "Core/Geometry/quad.h"

#include "influx_renderer.h"
#pragma comment(lib, "InfluxRenderer.lib")

#pragma region assimp
#include "foreign/assimp/assimp_helpers.h"
#if _DEBUG
#pragma comment(lib, "assimp-vc142-mtd.lib")
#else
#pragma comment(lib, "assimp-vc142-mt.lib")
#endif
#pragma endregion

namespace influx::application
{
	void renderthread::static_initialize()
	{
		auto app_run_args = application::get_instance().get_run_arguments();
		auto app_resource_directory = application::get_instance().get_resource_directory();

		// initialize renderer
		renderer::init_args args{};
		args.m_api_type = renderer::e_render_api::dx12;
		args.m_resource_dir = app_resource_directory;
		renderer::initialize(args);

		// initialize imgui backend
		if (app_run_args.m_enable_editor)
			renderer::initialize_imgui();

		// load meshes, textures & materials into the renderer
		uint32 num_submeshes{};
		vector<renderer::material_data> materials{};
		{
			// mesh assets
			{
				assimp_helpers::initialize();

				math::spheref bounding_sphere{};
				assimp_helpers::for_each_mesh_in(app_resource_directory + "/Meshes/Duolingo.fbx",
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
	}

	void renderthread::static_tick()
	{
		auto app_run_args = application::get_instance().get_run_arguments();

		// set up render & present arguments
		renderer::render_args render_args{};
		render_args.m_clear_colour = app_run_args.m_window_clear_colour;
		renderer::present_args present_args{};
		present_args.m_vsync = app_run_args.m_vsync;

		// setup scene proxy & hardcoded camera
		renderer::scene_proxy scene_proxy{};
		renderer::camera_proxy camera_proxy{};
		scene_proxy.m_cameras.push_back(camera_proxy);
		scene_proxy.m_cameras[0].m_fov = 90.0f;
		scene_proxy.m_cameras[0].m_near_plane = 0.01f;
		scene_proxy.m_cameras[0].m_far_plane = 1.0f;
		scene_proxy.m_cameras[0].m_position = m_camera_entity.m_transform.get_position();
		scene_proxy.m_cameras[0].m_forward = m_camera_entity.m_transform.get_forward();

		// update render proxies
		if (k_render_scene)
		{
			scene_proxy.m_meshes.resize(m_entities.size() * num_submeshes);
			if (k_jobify)
			{
				auto handles = async::dispatch_for(m_entities.size() * num_submeshes,
					[this, num_submeshes, &scene_proxy, materials](uint64 i)
					{
						uint32 entity_idx = i / num_submeshes;
						uint32 submesh_idx = i % num_submeshes;
						entity& entity = m_entities[entity_idx];

						renderer::mesh_proxy mesh{};
						mesh.m_name = "duolingo_mesh_" + std::to_string(submesh_idx);
						mesh.m_transform = entity.m_transform.get_matrix();
						mesh.m_per_instance_colour = materials[submesh_idx].m_albedo;

						scene_proxy.m_meshes[(entity_idx * num_submeshes) + submesh_idx] = mesh;
					});

				async::wait_for(handles);
			}
			else
			{
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
			}
		}

		renderer::render_to_window(k_render_scene ? &scene_proxy : nullptr, render_args, m_windowhandle, present_args);
	}

	void renderthread::static_cleanup()
	{
		renderer::cleanup();
	}
}