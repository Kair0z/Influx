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
		async::dispatch_for<file>(fbx_files, [this](const file& file)
		{
			assets::scene_load_args args{};
			assets::scene_data& scene_data = m_scenes[file.m_filename];
			assets::load_scene_file(file.m_path_full, scene_data, args);
		});

		// load pngs
		async::dispatch_for<file>(png_files, [this](const file& file)
		{
			assets::image_load_args args{};
			assets::image_data& texture_data = m_images[file.m_filename];
			assets::load_image_file(file.m_path_full, texture_data, args);
		});

		// load hlsls
		async::dispatch_for<file>(hlsl_files, [this](const file& file)
		{
			assets::shader_data& shader_data_vs = m_shaders[file.m_filename + "_vs"];
			assets::shader_data& shader_data_ps = m_shaders[file.m_filename + "_ps"];

			shader::compile_args compile_args{};
			compile_args.m_target = shader::e_shader_target::_6_6;
			compile_args.m_compile_debug = (_DEBUG) ? true : false;
			compile_args.m_pbd = true;
			compile_args.m_reflection = true;
			compile_args.m_defines = {};
			compile_args.m_pdb_folder = "D:/Git/Influx/int/shaderdebug/";

			compile_args.m_type = shader::e_shader_type::vs;
			compile_args.m_entrypoint = "VSMain";
			influx_assert(assets::load_shader_file(file.m_path_full, shader_data_vs, compile_args));

			compile_args.m_type = shader::e_shader_type::ps;
			compile_args.m_entrypoint = "PSMain";
			influx_assert(assets::load_shader_file(file.m_path_full, shader_data_ps, compile_args));
		});

		// wait for all jobs to complete
		async::wait_for_all();

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