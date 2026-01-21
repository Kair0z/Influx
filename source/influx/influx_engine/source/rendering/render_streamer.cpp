#include "engine_pch.h"
#include "render_streamer.h"

// influx::core
#include "core/material/material.h"
#include "core/scope.h"

// influx::engine
#include "assets/asset_manager.h"
#include "rendering/render_manager.h"

// influx::shader
#include "influx_shader.h"

// influx::import
#include "influx_import.h"

// influx::renderer
#include "influx_renderer.h"

namespace influx::engine
{
#pragma region translation
	void translate(const imp::scene_data::mesh& imp_data, renderer::mesh_data<renderer::vertex_data>& out_data);
	void translate(const imp::shader_data& imp_data, renderer::shader_data& out_data);
	void translate(const imp::image_data& imp_data, renderer::texture2D_data& out_data);
	void translate(const shader::compile_output& shader_data, renderer::shader_data& out_data);
#pragma endregion

	void render_streamer::stream(const asset_manager& content)
	{
		influx_scope("render_stream");
		stream_images(content);
		stream_shaders(content);
		stream_meshes(content);
		stream_cubemaps(content);
	}
	bool render_streamer::has_shader_loaded(const shader::shader_signature& signature) const
	{
		return influx::renderer::has_shader(renderer::make_shader_id(signature));
	}
	bool render_streamer::has_mesh_loaded(const string& name) const
	{
		return influx::renderer::has_mesh(name);
	}
	bool render_streamer::has_texture_loaded(const string& name) const
	{
		return influx::renderer::has_texture(name);
	}
	bool render_streamer::has_cubemap_loaded(const string& name) const
	{
		return influx::renderer::has_cubemap(name);
	}

#pragma region translation layer
	void translate(const imp::scene_data::mesh& imp_data, renderer::mesh_data<renderer::vertex_data>& out_data)
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

	void translate(const imp::image_data& imp_data, renderer::texture2D_data& out_data)
	{
		out_data.m_pixels.resize(imp_data.m_pixels.size());

		for (uint64 i = 0u; i < imp_data.m_pixels.size(); ++i)
		{
			out_data.m_pixels[i] = imp_data.m_pixels[i];
		}

		out_data.m_width = imp_data.m_dimensions.x;
	}

	void translate(const imp::cubemap_data& imp_data, renderer::cubemap_data& out_data)
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

		out_data.m_type = shader_data.m_signature.get_shader_type();
		out_data.m_reflection = shader_data.m_reflection;
	}
#pragma endregion

	// staging buffers
	static renderer::shader_data m_shader_data{};
	static renderer::texture2D_data m_tex_data{};
	static renderer::cubemap_data m_texcube_data{};
	static renderer::mesh_data<renderer::vertex_data> m_mesh_data{};
	static material m_material_data{};

	void render_streamer::stream_shaders(const asset_manager& content)
	{
		influx_scope("render_stream_shaders");
	}

	void render_streamer::stream_images(const asset_manager& content)
	{
		render_manager& renderman = engine::get_instance().get_renderer();

		influx_scope("render_stream_images");
	}

	void render_streamer::stream_cubemaps(const asset_manager& content)
	{
		influx_scope("render_stream_cubemaps");
	}

	void render_streamer::stream_meshes(const asset_manager& content)
	{
		influx_scope("render_stream_meshes");
	}
}