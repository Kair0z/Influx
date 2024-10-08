#include "app_pch.h"
#include "renderer.h"

// influx::application
#include "application/application_backend.h"
#include "content/content_manager.h"
#include "editor/editor.h"

// influx::renderer
#include "influx_renderer.h"
#include "influx_renderer/target.h"

// imgui
#include "imgui/imgui.h"

namespace influx::application
{
	renderer::renderer(platform::window_handle window_handle)
		: m_window_handle{ window_handle }
	{
		// create renderer
		influx::renderer::init_args render_init_args{};
		render_init_args.m_api_type = influx::renderer::e_render_api::dx12;
		// render_init_args.m_api_type = influx::renderer::e_render_api::vulkan;
		influx::renderer::initialize(render_init_args);

		// window render target:
		mp_window_target = influx::renderer::get_window_target(m_window_handle);

		// scene render target:
		influx::renderer::target_create_args target_args{};
		target_args.m_has_depth_stencil = true;
		target_args.m_width = mp_window_target->get_width();
		target_args.m_heigth = mp_window_target->get_height();
		mp_scene_color_target = influx::renderer::create_target(target_args);

		// imgui
		application::get_editor()->subscribe([this]()
		{
			ImGui::Begin("Renderer");
			const auto mem_info = influx::renderer::get_memory_info();
			const float percentage = (float)mem_info.m_gpu_usage / mem_info.m_gpu_budget;
			ImGui::Text("Gpu Memory %u / %u (%f)", mem_info.m_gpu_usage, mem_info.m_gpu_budget, percentage);
			ImGui::End();
		});
	}

	// renders the scene
	void renderer::render(const influx::renderer::scene& scene)
	{
		// update the window target
		mp_window_target = influx::renderer::get_window_target(m_window_handle);

		influx::renderer::draw_scene(scene, *mp_scene_color_target);

		// draw editor on top of scene
		if (application::get_editor())
		{
			ImDrawData* drawdata = application::get_editor()->get_imgui_drawdata();
			if (drawdata)
			{
				influx::renderer::draw_imgui(drawdata, *mp_scene_color_target);
			}	
		}

		influx::renderer::copy_target(*mp_scene_color_target, *mp_window_target);

		influx::renderer::present_args present_args{};
		present_args.m_vsync = true;
		influx::renderer::present_swapchain(present_args);
	}

	// mesh geometry
	inline bool load_to_renderer(const assets::scene_data::mesh& mesh, const string& name)
	{
		const assets::scene_data::mesh& mesh = asset.second.m_meshes[0]; // gets the first mesh
		const std::string& name = asset.first;

		influx::renderer::mesh_data mesh_data{};
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
		influx::renderer::load(name, mesh_data);
		return true;
	}

	// shaders
	inline bool load_to_renderer(const assets::shader_data& shader, const string& name)
	{
		influx::renderer::shader_data shader_data{};
		shader_data.m_bytecode = shader.m_compile_result.m_bytecode;
		shader_data.m_type = shader.m_type;
		shader_data.m_reflection = shader.m_compile_result.m_reflection;

		influx::renderer::load(name, shader_data);
		return true;
	}

	// textures
	inline bool load_to_renderer(const assets::image_data& image, const string& name)
	{
		influx::renderer::texture_data tex_data{};
		tex_data.m_pixels = image.m_pixels;
		tex_data.m_width = image.m_dimensions.x;
		influx::renderer::load(name, tex_data);
		return true;
	}

	// loads application asset data into renderer (textures/meshes/shaders)
	void renderer::load_render_assets(content_manager* cont_man)
	{
		// GEOMETRY
		for (const auto& asset : cont_man->get_scenes())
		{
			// todo: for now, only loading the first mesh...
			load_to_renderer(asset.second.m_meshes[0], asset.first);
		}

		// TEXTURES
		for (const auto& asset : cont_man->get_images())
		{
			load_to_renderer(asset.second, asset.first);
		}

		// SHADERS
		// todo: for now only 2 shaders are necessary
		#if 0
		for (const auto& asset : cont_man->get_shaders())
		{
			load_to_renderer(asset.second, asset.first);
		}
		#else
		const assets::shader_data& vs_shader = cont_man->get_shaders().at("shaders_vs");
		const assets::shader_data& ps_shader = cont_man->get_shaders().at("shaders_ps");
		load_to_renderer(vs_shader, "shaders_vs");
		load_to_renderer(ps_shader, "shaders_ps");
		#endif

		// MATERIALS
		{
			influx::renderer::material mat_transistor{};
			mat_transistor.m_basecolor = colour::k_white;
			mat_transistor.m_tex_albedo = "T_Sword_Opaque_BC";
			mat_transistor.m_tex_normal = "T_Sword_Opaque_N";
			mat_transistor.m_tex_roughness = "T_Sword_Opaque_N";
			mat_transistor.m_tex_special = "T_Sword_Opaque_N";
			influx::renderer::load("mat_transistor", mat_transistor);
		}

		// load engine geometry meshes
		// PLANE
		{
			math::vectorf3 positions[4u]
			{
				{ 1.0f, 0.0f, 1.0f },
				{ -1.0f, 0.0f, 1.0f },
				{ 1.0f, 0.0f, -1.0f },
				{ -1.0f, 0.0f, -1.0f }
			};
			math::vectorf4 colours[4u]
			{
				{ 1.0f, 0.0f, 0.0f, 1.0f },
				{ 0.0f, 1.0f, 0.0f, 1.0f },
				{ 0.0f, 0.0f, 1.0f, 1.0f },
				{ 1.0f, 1.0f, 0.0f, 1.0f }
			};
			math::vectorf2 uvs[4u]
			{
				{ 0.0f, 0.0f },
				{ 1.0f, 0.0f },
				{ 1.0f, 1.0f },
				{ 0.0f, 1.0f }
			};
			math::vectorf3 normals[4u]
			{
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f }
			};

			influx::renderer::mesh_data mesh_data{};
			for (uint8 i = 0u; i < 4u; ++i)
			{
				mesh_data.m_vertices.push_back
				({
					.m_position{positions[i]},
					.m_colour{colours[i]},
					.m_normal{normals[i]},
					.m_texcoords{uvs[i]}
				});
			}
			
			mesh_data.m_indices.push_back(0u);
			mesh_data.m_indices.push_back(2u);
			mesh_data.m_indices.push_back(1u);
			mesh_data.m_indices.push_back(2u);
			mesh_data.m_indices.push_back(3u);
			mesh_data.m_indices.push_back(1u);

			influx::renderer::load("engine_plane", mesh_data);
		}	
	}
}