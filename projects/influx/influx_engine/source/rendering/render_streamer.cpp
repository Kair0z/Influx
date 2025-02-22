#include "engine_pch.h"
#include "render_streamer.h"

// influx::core
#include "core/material/material.h"

// influx::engine
#include "content/content_manager.h"

// influx::shader
#include "influx_shader.h"

// influx::import
#include "influx_import.h"

// influx::renderer
#include "influx_renderer.h"

namespace influx::engine
{
#pragma region translation
	void translate(const imp::scene_data::mesh& imp_data, renderer::mesh_data& out_data);
	void translate(const imp::shader_data& imp_data, renderer::shader_data& out_data);
	void translate(const imp::image_data& imp_data, renderer::texture_data& out_data);
	void translate(const shader::compile_output& shader_data, renderer::shader_data& out_data);
#pragma endregion

	void render_streamer::stream(const content_manager& content)
	{
		stream_images(content);
		stream_shaders(content);
		stream_meshes(content);
		stream_cubemaps(content);
	}
	bool render_streamer::has_shader_loaded(const shader::shader_signature& signature) const
	{
		return influx::renderer::has_shader(signature);
	}
	bool render_streamer::has_mesh_loaded(const string& name) const
	{
		return influx::renderer::has_mesh(name);
	}
	bool render_streamer::has_texture_loaded(const string& name) const
	{
		return influx::renderer::has_texture(name);
	}
	bool render_streamer::has_texturecube_loaded(const string& name) const
	{
		return influx::renderer::has_texturecube(name);
	}
	void* render_streamer::get_loaded_texture_id(const string& name) const
	{
		if (has_texture_loaded(name))
		{
			return influx::renderer::get_imgui_texture_id(name);
		}
		else
		{
			return 0u;
		}
	}

#pragma region content_streaming
#pragma region translation layer
	void translate(const imp::scene_data::mesh& imp_data, renderer::mesh_data& out_data)
	{
		out_data.m_indices.resize(imp_data.m_indices.size());
		out_data.m_vertices.resize(imp_data.m_positions.size());

		for (uint64 i = 0u; i < imp_data.m_positions.size(); ++i)
		{
			out_data.m_vertices[i].m_position = imp_data.m_positions[i];
			out_data.m_vertices[i].m_normal = imp_data.m_normals[i];
			out_data.m_vertices[i].m_texcoords = imp_data.m_uvs[i];
			// ...
		}

		for (uint64 i = 0u; i < imp_data.m_indices.size(); ++i)
		{
			out_data.m_indices[i] = imp_data.m_indices[i];
		}
	}

	renderer::shader_data translate(const imp::shader_data& imp_data)
	{
		renderer::shader_data result{};
		result.m_bytecode.resize(imp_data.m_compile_result.m_bytecode.size());

		for (uint64 i = 0u; i < imp_data.m_compile_result.m_bytecode.size(); ++i)
		{
			result.m_bytecode[i] = imp_data.m_compile_result.m_bytecode[i];
		}

		result.m_type = imp_data.m_type;
		result.m_reflection = imp_data.m_compile_result.m_reflection;
		return result;
	}

	void translate(const imp::shader_data& imp_data, renderer::shader_data& out_data)
	{
		out_data.m_bytecode.resize(imp_data.m_compile_result.m_bytecode.size());

		for (uint64 i = 0u; i < imp_data.m_compile_result.m_bytecode.size(); ++i)
		{
			out_data.m_bytecode[i] = imp_data.m_compile_result.m_bytecode[i];
		}

		out_data.m_type = imp_data.m_type;
		out_data.m_reflection = imp_data.m_compile_result.m_reflection;
	}

	void translate(const imp::image_data& imp_data, renderer::texture_data& out_data)
	{
		out_data.m_pixels.resize(imp_data.m_pixels.size());

		for (uint64 i = 0u; i < imp_data.m_pixels.size(); ++i)
		{
			out_data.m_pixels[i] = imp_data.m_pixels[i];
		}

		out_data.m_width = imp_data.m_dimensions.x;
	}

	void translate(const imp::cubemap_data& imp_data, renderer::texturecube_data& out_data)
	{
		out_data.m_pixels.resize(imp_data.m_pixels.size());
		for (uint64 i = 0u; i < imp_data.m_pixels.size(); ++i)
		{
			out_data.m_pixels[i] = imp_data.m_pixels[i];
		}
		out_data.m_width = imp_data.m_dimensions.x;
		out_data.m_height = imp_data.m_dimensions.y;
	}

	void translate(const shader::compile_output& shader_data, renderer::shader_data& out_data)
	{
		out_data.m_bytecode.resize(shader_data.m_bytecode.size());

		for (uint64 i = 0u; i < shader_data.m_bytecode.size(); ++i)
		{
			out_data.m_bytecode[i] = shader_data.m_bytecode[i];
		}

		out_data.m_type = shader_data.m_signature.m_type;
		out_data.m_reflection = shader_data.m_reflection;
	}
#pragma endregion

	// staging buffers
	static renderer::shader_data m_shader_data{};
	static renderer::texture_data m_tex_data{};
	static renderer::texturecube_data m_texcube_data{};
	static renderer::mesh_data m_mesh_data{};
	static material m_material_data{};

	void render_streamer::stream_shaders(const content_manager& content)
	{
		for (const auto& asset : content.get_shaders())
		{
			if (asset.second.is_loaded())
			{
				const imp::shader_data shader_data = asset.second.m_resource;
				const shader::shader_signature& signature = shader_data.m_signature;

				const time::point render_load_time = renderer::get_time_loaded_shader(signature);
				if (!renderer::has_shader(signature) || asset.second.m_time_loadend > render_load_time)
				{
					// if load time is newer than previous render load time, reload
					influx::renderer::load(signature, translate(shader_data), true);
				}
			}
		}
	}

	void render_streamer::stream_images(const content_manager& content)
	{
		for (const auto& asset : content.get_images())
		{
			if (renderer::has_texture(asset.first) == false)
			{
				if (asset.second.is_loaded())
				{
					translate(asset.second.m_resource, m_tex_data);
					influx::renderer::load(asset.first, m_tex_data);
				}
			}
		}
	}

	void render_streamer::stream_cubemaps(const content_manager& content)
	{
		for (const auto& asset : content.get_cubemaps())
		{
			if (renderer::has_texturecube(asset.first) == false)
			{
				if (asset.second.is_loaded())
				{
					translate(asset.second.m_resource, m_texcube_data);
					influx::renderer::load(asset.first, m_texcube_data);
				}
			}
		}
	}

	void render_streamer::stream_meshes(const content_manager& content)
	{
		// content meshes
		for (const auto& asset : content.get_scenes())
		{
			if (asset.second.is_loaded())
			{
				for (uint32 i = 0u; i < asset.second.m_resource.get_num_meshes(); ++i)
				{
					const imp::scene_data::mesh& mesh = asset.second.m_resource.get_mesh(i);
					const string name = asset.first + "_" + std::to_string(i);

					if (renderer::has_mesh(name) == false)
					{
						translate(mesh, m_mesh_data);
						influx::renderer::load(name, m_mesh_data);
					}
				}
			}
		}

		// inline meshes
		if (renderer::has_mesh("engine_plane") == false)
		{
			influx::renderer::load("eplane", renderer::get_inline_mesh_plane());
		}
		if (renderer::has_mesh("engine_box") == false)
		{
			influx::renderer::load("ebox", renderer::get_inline_mesh_box());
		}
		if (renderer::has_mesh("engine_sphere") == false)
		{
			influx::renderer::load("esphere", renderer::get_inline_mesh_sphere());
		}
	}
#pragma endregion
}