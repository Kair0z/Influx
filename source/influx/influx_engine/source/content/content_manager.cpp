#include "engine_pch.h"
#include "content_manager.h"

// influx::core
#include "core/log.h"
#include "core/time.h"
#include "core/file.h"

// influx::engine
#include "file/engine_files.h"
#include "editor/editor_manager.h"
#include "imgui/imgui.h"

// influx::async
#include "influx_async.h"

// influx::import
#include "influx_import.h"

// influx::shader
#include "influx_shader.h"

namespace influx::engine
{
	class content_ui final : public editor::editor_window
	{
	public:
		virtual void on_run() override
		{
			static content_manager& content = get_engine()->get_content();
			static render_manager& renderer = get_engine()->get_renderer();

			set_name("engine:content");

			if (ImGui::Button("recomp_shaders"))
			{
				for (auto& pair : content.touch_shaders())
				{
					pair.second.reload();
				}
			}

			if (ImGui::BeginTabBar("content"))
			{
				if (ImGui::BeginTabItem("scenes"))
				{
					// "scene:filepath"
					for (const auto& pair : content.get_scenes())
					{
						const string& name = pair.first;
						const scene_asset& scene_asset = pair.second;

						if (scene_asset.is_loaded() && scene_asset.is_engine())
						{
							if (ImGui::TreeNode(name.c_str(), "scene: %s - ms : % f", name.c_str(), scene_asset.get_load_ms()))
							{
								for (uint32 i = 0u; i < scene_asset.get_resource().get_num_meshes(); ++i)
								{
									const string& mesh_name = name + "_" + to_string(i);
									ImGui::Text(mesh_name.c_str());
								}
								ImGui::TreePop();
							}
						}
					}
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("textures"))
				{
					const float size = 50.0f;

					if (ImGui::BeginTable("ed_texture_grid", 4u))
					{
						for (const auto& pair : content.get_images())
						{
							if (pair.second.is_loaded() && pair.second.is_engine())
							{
								ImGui::TableNextColumn();

								const string& name = pair.first;
								const image_asset& image = pair.second;
								const math::vectoru2& image_dims = image.m_resource.m_dimensions;
								ImGui::TextWrapped("%s", name.c_str());
							}
						}
						ImGui::EndTable();
					}

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("shaders"))
				{
					// "shader:filepath"
					for (const auto& pair : content.get_shaders())
						if (pair.second.is_loaded() && pair.second.is_engine())
							ImGui::Text("shader:%s - ms:%f", pair.first.c_str(), pair.second.get_load_ms());
					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}
		}
	};

	static const load_args<e_asset_type::scene> k_default_scene_import_args
	{
		.m_pre_scale = 1.0f,
		.m_bake_transforms = false
	};

	static const load_args<e_asset_type::image> k_default_image_import_args
	{
		
	};

	content_manager::content_manager(engine* engine)
	{
		// immediately start kicking loading
		load_engine_assets(engine);

		editor::editor_manager::static_window<content_ui>("content");
	}

	content_manager::~content_manager()
	{
	}

	const umap<string, scene_asset>& content_manager::get_scenes() const
	{
		return m_scenes;
	}
	const umap<string, image_asset>& content_manager::get_images() const
	{
		return m_images;
	}
	const umap<string, shader_asset>& content_manager::get_shaders() const
	{
		return m_shaders;
	}
	const umap<string, cubemap_asset>& content_manager::get_cubemaps() const
	{
		return m_cubemaps;
	}
	umap<string, shader_asset>& content_manager::touch_shaders()
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

	void content_manager::load(const string& path)
	{
		file as_file = file(path);
		e_asset_type asset_type = e_asset_type::count;

		if (as_file.m_extension == ".fbx") asset_type = e_asset_type::scene;
		if (as_file.m_extension == ".png") asset_type = e_asset_type::scene;
		if (as_file.m_extension == ".hlsl") asset_type = e_asset_type::scene;

		switch (asset_type)
		{
		case e_asset_type::scene: 
			m_scenes[as_file.m_filename].load(path, k_default_scene_import_args);
			break;
		}
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
		const vector<file> fbx_files = get_files_in_directory(root.m_path_full, true, ".fbx");
		const vector<file> obj_files = get_files_in_directory(root.m_path_full, true, ".obj");
		const vector<file> png_files = get_files_in_directory(root.m_path_full, true, ".png");
		const vector<file> hlsl_files = get_files_in_directory(root.m_path_full, true, ".hlsl");

		// load cubemap (hack)
		{
			imp::cubemap_load_args args{};
			stat_array<string, 6u> cubemap_side_files{};
			uint32 i = 0u;
			for (const file& png : png_files)
			{
				if (str::contains(png.m_filename, "graycloud"))
				{
					cubemap_side_files[i++] = png.m_path_full;
				}
			}

			cubemap_asset& item = m_cubemaps["graycloud"];
			args.m_hacky_paths = &cubemap_side_files;
			item.load(cubemap_side_files[0], args);
		}
		
		// load fbxs & objs
		async::dispatch_for<file>(obj_files, [this](const file& file)
		{
			imp::scene_load_args args{};
			args.m_bake_transforms = true;
			args.m_pre_scale = 1;
			scene_asset& item = m_scenes[file.m_filename];
			item.load(file, args);
		});
		async::dispatch_for<file>(fbx_files, [this](const file& file)
		{
			imp::scene_load_args args{};
			args.m_bake_transforms = true;
			args.m_pre_scale = 1;
			scene_asset& item = m_scenes[file.m_filename];
			item.load(file, args);
		});

		// load hlsls
		static shader::compile_args compile_args{};
		if (origin == e_asset_origin::engine)
		{
			compile_args.m_include_folder = root.m_path_full + "/engine/shaders/";
		}
		
		compile_args.m_signature.m_target = shader::e_shader_target::_6_6;
		compile_args.m_reflection_enabled = true;
		compile_args.m_defines = {};
#if INFLUX_DEBUG
		compile_args.set_debug_level(true);
		compile_args.m_pbd_enabled = true;
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
				shader_asset& vs_item = m_shaders[file.m_filename + "_vs"];
				compile_args.m_signature.m_type = shader::e_shader_type::vs;
				compile_args.m_signature.m_entrypoint = "main_vs";
				compile_args.m_signature.cache_id();
				vs_item.load(file, { compile_args });
			}

			if (str::contains(file_content, "[shader(\"pixel\")]", false))
			{
				shader_asset& ps_item = m_shaders[file.m_filename + "_ps"];
				compile_args.m_signature.m_type = shader::e_shader_type::ps;
				compile_args.m_signature.m_entrypoint = "main_ps";
				compile_args.m_signature.cache_id();
				ps_item.load(file, { compile_args });
			}

			if (str::contains(file_content, "[shader(\"compute\")]", false))
			{
				shader_asset& cs_item = m_shaders[file.m_filename + "_cs"];
				compile_args.m_signature.m_type = shader::e_shader_type::cs;
				compile_args.m_signature.m_entrypoint = "main_cs";
				compile_args.m_signature.cache_id();
				cs_item.load(file, { compile_args });
			}
		});

		// load pngs
		async::dispatch_for<file>(png_files, [this](const file& file)
		{
			imp::image_load_args args{};
			image_asset& item = m_images[file.m_filename];
			item.load(file, args);
		});
	}
	
	void content_manager::write_native(const image_asset& asset)
	{
		
	}
}