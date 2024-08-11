#include "app_pch.h"
#include "content_manager.h"

// core
#include "core/log.h"

// async
#include "influx_async.h"

// shader
#include "influx_shader.h"

namespace influx::application
{
	content_manager::content_manager(const string& resource_dir)
	{
		logn("loading assets ... :3");
		time::point time_before_load = time::get_now();

		vector<file> fbx_files = get_files_in_directory(resource_dir, true, ".fbx");
		vector<file> png_files = get_files_in_directory(resource_dir, true, ".png");
		vector<file> hlsl_files = get_files_in_directory(resource_dir, true, ".hlsl");

		// load fbxs
		for (uint64 i = 0u; i < fbx_files.size(); ++i)
		{
			const file& file = fbx_files[i];
			assets::scene_load_args args{};
			assets::scene_data& scene_data = m_scenes[file.m_filename];
			assets::load_scene_file(file.m_path_full, scene_data, args);
		};

		// load pngs
		for (uint64 i = 0u; i < png_files.size(); ++i)
		{
			const file& file = png_files[i];
			assets::image_load_args args{};
			assets::image_data& texture_data = m_images[file.m_filename];
			assets::load_image_file(file.m_path_full, texture_data, args);
		};

		// load hlsls
		for(uint64 i = 0u; i < hlsl_files.size(); ++i)
		{
			const file& file = hlsl_files[i];
			assets::shader_data& shader_data_vs = m_shaders[file.m_filename + "_vs"];
			assets::shader_data& shader_data_ps = m_shaders[file.m_filename + "_ps"];

			shader::compile_args compile_args{};
			compile_args.m_target = shader::e_shader_target::_6_2;
			compile_args.m_compile_debug = (_DEBUG) ? true : false;
			compile_args.m_pbd = false;
			compile_args.m_reflection = true;
			compile_args.m_defines = {};

			compile_args.m_type = shader::e_shader_type::vs;
			compile_args.m_entrypoint = "VSMain";
			assets::load_shader_file(file.m_path_full, shader_data_vs, compile_args);

			compile_args.m_type = shader::e_shader_type::ps;
			compile_args.m_entrypoint = "PSMain";
			assets::load_shader_file(file.m_path_full, shader_data_ps, compile_args);
		};

		logn("finished loading assets in {} seconds", time::get_ms_since<float>(time_before_load) * 0.001f);
	}

	const map<string, assets::scene_data>& content_manager::get_scenes() const
	{
		return m_scenes;
	}

	const map<string, assets::image_data>& content_manager::get_images() const
	{
		return m_images;
	}

	const map<string, assets::shader_data>& content_manager::get_shaders() const
	{
		return m_shaders;
	}

}