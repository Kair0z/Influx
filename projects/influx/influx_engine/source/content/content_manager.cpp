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
	content_manager::content_manager(engine* engine)
	{
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

	void content_manager::load_assets(engine* engine, e_asset_origin origin, const file& root)
	{	
		vector<file> fbx_files = get_files_in_directory(root.m_path_full, true, ".fbx");
		vector<file> png_files = get_files_in_directory(root.m_path_full, true, ".png");
		vector<file> hlsl_files = get_files_in_directory(root.m_path_full, true, ".hlsl");

		// load fbxs
		async::dispatch_for<file>(fbx_files, [this](const file& file)
		{
			imp::scene_load_args args{};
			scene_item& item = m_scenes[file.m_filename];
			item.set_loadstate(e_load_state::loading);
			imp::load_scene_file(file.m_path_full, item.m_resource, args);
			item.set_loadstate(e_load_state::loaded);
		});

		// load hlsls
		const auto& interm_dir = get_engine_directory(engine_directory::intermediate);
		async::dispatch_for<file>(hlsl_files, [this, root, interm_dir](const file& file)
		{
			shader_item& vs_item = m_shaders[file.m_filename + "_vs"];
			shader_item& ps_item = m_shaders[file.m_filename + "_ps"];

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
			compile_args.m_entrypoint = "main_vs";
			vs_item.set_loadstate(e_load_state::loading);
			influx_assert(imp::load_shader_file(file.m_path_full, vs_item.m_resource, compile_args));
			vs_item.set_loadstate(e_load_state::loaded);

			compile_args.m_type = shader::e_shader_type::ps;
			compile_args.m_entrypoint = "main_ps";
			ps_item.set_loadstate(e_load_state::loading);
			influx_assert(imp::load_shader_file(file.m_path_full, ps_item.m_resource, compile_args));
			ps_item.set_loadstate(e_load_state::loaded);
		});

		// load pngs
		async::dispatch_for<file>(png_files, [this](const file& file)
		{
			imp::image_load_args args{};
			image_item& item = m_images[file.m_filename];
			item.set_loadstate(e_load_state::loading);
			imp::load_image_file(file.m_path_full, item.m_resource, args);
			item.set_loadstate(e_load_state::loaded);
		});
	}
}