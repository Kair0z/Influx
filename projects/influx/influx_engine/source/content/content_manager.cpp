#include "engine_pch.h"
#include "content_manager.h"

// influx::core
#include "core/log.h"
#include "core/time.h"

// influx::engine
#include "file/engine_files.h"

// influx::async
#include "influx_async.h"

// influx::import
#include "influx_import.h"

// influx::shader
#include "influx_shader.h"

namespace influx::engine
{
	imp::scene_data content_manager::load_scene_data(const string& path, const imp::scene_load_args& args)
	{
		imp::scene_data data{};
		imp::load_scene_file(path, data, args);
		return data;
	}
	imp::image_data content_manager::load_image_data(const string& path, const imp::image_load_args& args)
	{
		imp::image_data data{};
		imp::load_image_file(path, data, args);
		return data;
	}
	imp::shader_data content_manager::load_shader_data(const string& path, const shader::compile_args& args)
	{
		imp::shader_data result{};
		bool success = imp::load_shader_file(path, result, args);
		return result;
	}

	content_manager::content_manager(engine* engine)
	{
		// immediately start kicking loading
		load_engine_assets(engine);
	}

	content_manager::~content_manager()
	{
	}

	const umap<string, content_manager::scene_item>& content_manager::get_scenes() const
	{
		return m_scenes;
	}

	const umap<string, content_manager::image_item>& content_manager::get_images() const
	{
		return m_images;
	}

	const umap<string, content_manager::shader_item>& content_manager::get_shaders() const
	{
		return m_shaders;
	}

	umap<string, content_manager::shader_item>& content_manager::touch_shaders()
	{
		return m_shaders;
	}

	void content_manager::load_engine_assets(engine* engine)
	{
		m_start_engine_resources = time::get_now();

		logn("loading engine resources ...");
		const auto engine_assets_dir = get_engine_directory(engine_directory::assets);
		load_assets(engine, e_asset_origin::engine, engine_assets_dir);
	}

	void content_manager::load_game_assets(const string& game_name, engine* engine)
	{
		logn("loading {} resources ...", game_name.c_str());
		const auto game_assets_dir = get_game_directory(game_name, game_directory::assets);
		load_assets(engine, e_asset_origin::game, game_assets_dir);
	}

	void content_manager::update_filechanges()
	{
		for (auto& pair : touch_shaders())
		{
			const string& path = pair.second.m_path;
			
		}
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
			args.m_bake_transforms = false;
			args.m_pre_scale = 1.0f;
			scene_item& item = m_scenes[file.m_filename];
			item.load(file.m_path_full, args);
		});

		// load hlsls
		static shader::compile_args compile_args{};
		if (origin == e_asset_origin::engine)
		{
			compile_args.m_include_folder = root.m_path_full + "/engine/shaders/";
		}
		
		compile_args.m_signature.m_target = shader::e_shader_target::_6_6;
		compile_args.m_reflection = true;
		compile_args.m_defines = {};
#if INFLUX_DEBUG
		compile_args.m_compile_debug = true;
		compile_args.m_pbd = true;
		compile_args.m_pdb_folder = get_engine_directory(engine_directory::shaderpdb).m_path_full.c_str();
#else
		compile_args.m_compile_debug = false;
		compile_args.m_pbd = false;
#endif

		async::dispatch_for<file>(hlsl_files, [this, root](const file& file)
		{
#if INFLUX_DEBUG
			compile_args.m_pdb_filename = file.m_filename;
#endif
			compile_args.m_signature.m_filename = file.m_filename;

			const string& file_content = textfile::read_all(file.m_path_full);
			if (str::contains(file_content, "[shader(\"vertex\")]", false))
			{
				shader_item& vs_item = m_shaders[file.m_filename + "_vs"];
				compile_args.m_signature.m_type = shader::e_shader_type::vs;
				compile_args.m_signature.m_entrypoint = "main_vs";
				compile_args.m_signature.cache_id();
				vs_item.load(file.m_path_full, compile_args);
			}

			if (str::contains(file_content, "[shader(\"pixel\")]", false))
			{
				shader_item& ps_item = m_shaders[file.m_filename + "_ps"];
				compile_args.m_signature.m_type = shader::e_shader_type::ps;
				compile_args.m_signature.m_entrypoint = "main_ps";
				compile_args.m_signature.cache_id();
				ps_item.load(file.m_path_full, compile_args);
			}

			if (str::contains(file_content, "[shader(\"compute\")]", false))
			{
				shader_item& cs_item = m_shaders[file.m_filename + "_cs"];
				compile_args.m_signature.m_type = shader::e_shader_type::cs;
				compile_args.m_signature.m_entrypoint = "main_cs";
				compile_args.m_signature.cache_id();
				cs_item.load(file.m_path_full, compile_args);
			}
		});

		// load pngs
		async::dispatch_for<file>(png_files, [this](const file& file)
		{
			imp::image_load_args args{};
			image_item& item = m_images[file.m_filename];
			item.load(file.m_path_full, args);
		});
	}
}