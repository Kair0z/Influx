#include "engine_pch.h"
#include "content_manager.h"

// influx::core
#include "core/log.h"
#include "core/time.h"

// influx::async
#include "influx_async.h"

// influx::import
#include "influx_import.h"

// influx::shader
#include "influx_shader.h"

namespace influx::engine
{
	content_manager::content_manager(engine* engine)
	{
		// kick-off loading thread
		m_loading_thread = thread([this, engine]()
		{
			while (engine->is_quit() == false)
			{
				if (m_is_loading)
				{
					// wait for all jobs to complete
					async::wait_for_all();

					float seconds_since_load = time::get_ms_since<float>(m_start_engine_resources) * 0.001f;
					logonce(e_log_category::normal, "finished resources in {} seconds", seconds_since_load);

					m_is_loading = false;
				}
			}
		});
	}

	content_manager::~content_manager()
	{
		if (m_loading_thread.joinable())
			m_loading_thread.join();
	}

	const map<string, content_manager::scene_item>& content_manager::get_scenes() const
	{
		return m_scenes;
	}

	const map<string, content_manager::image_item>& content_manager::get_images() const
	{
		return m_images;
	}

	const map<string, content_manager::shader_item>& content_manager::get_shaders() const
	{
		return m_shaders;
	}

	void content_manager::load_engine_assets(engine* engine)
	{
		m_start_engine_resources = time::get_now();

		logn("loading engine resources ...");
		load_assets(engine, e_asset_origin::engine, 
			engine->get_engine_directory(engine::e_directory::assets));
	}

	void content_manager::load_game_assets(const string& game_name, engine* engine)
	{
		logn("loading {} resources ...", game_name.c_str());
		load_assets(engine, e_asset_origin::game, 
			engine->get_game_directory(game_name, engine::e_game_directory::assets));
	}

	void content_manager::load_assets(engine* engine, e_asset_origin origin, const file& root)
	{	
		vector<file> fbx_files = get_files_in_directory(root.m_path_full, true, ".fbx");
		vector<file> png_files = get_files_in_directory(root.m_path_full, true, ".png");
		vector<file> hlsl_files = get_files_in_directory(root.m_path_full, true, ".hlsl");

		// load fbxs
		async::dispatch_for<file>(fbx_files, [this](const file& file)
		{
			imp::scene_load_args args{};
			imp::scene_data& scene_data = m_scenes[file.m_filename].m_resource;
			imp::load_scene_file(file.m_path_full, scene_data, args);
		});

		// load pngs
		async::dispatch_for<file>(png_files, [this](const file& file)
		{
			imp::image_load_args args{};
			imp::image_data& texture_data = m_images[file.m_filename].m_resource;
			imp::load_image_file(file.m_path_full, texture_data, args);
		});

		// load hlsls
		const auto& interm_dir = engine->get_engine_directory(engine::e_directory::intermediate);
		async::dispatch_for<file>(hlsl_files, [this, root, interm_dir](const file& file)
		{
			imp::shader_data& shader_data_vs = m_shaders[file.m_filename + "_vs"].m_resource;
			imp::shader_data& shader_data_ps = m_shaders[file.m_filename + "_ps"].m_resource;

			shader::compile_args compile_args{};
			compile_args.m_target = shader::e_shader_target::_6_6;
#if INFLUX_DEBUG
			compile_args.m_compile_debug = true;
#else
			compile_args.m_compile_debug = false;
#endif
			compile_args.m_pbd = true;
			compile_args.m_reflection = true;
			compile_args.m_defines = {};
			compile_args.m_pdb_folder = interm_dir.m_path_full + "/shaderdebug/";
			compile_args.m_include_folder = root.m_path_full + "/shaders/include/";

			compile_args.m_type = shader::e_shader_type::vs;
			compile_args.m_entrypoint = "VSMain";
			influx_assert(imp::load_shader_file(file.m_path_full, shader_data_vs, compile_args));

			compile_args.m_type = shader::e_shader_type::ps;
			compile_args.m_entrypoint = "PSMain";
			influx_assert(imp::load_shader_file(file.m_path_full, shader_data_ps, compile_args));
		});

		m_is_loading = true;
	}
}