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

	static const load_args<e_asset_type::shader> k_default_shader_import_args
	{ {
		.m_signature{},
		.m_defines{},
		.m_pdb_folder{},
		.m_pdb_filename{},
		.m_include_folder{},
#if INFLUX_DEBUG
		.m_debug_level = shader::e_compile_debug_level::debug,
#else
		.m_debug_level = shader::e_compile_debug_level::release,
#endif
		.m_reflection_enabled = true,
		.m_pbd_enabled = true
	}};

	static const load_args<e_asset_type::scene> k_default_scene_import_args
	{
		.m_pre_scale = 1.0f,
		.m_bake_transforms = false
	};

	static const load_args<e_asset_type::image> k_default_image_import_args
	{
		
	};

	content_manager::content_manager()
	{
		// immediately start kicking engine loading (may as well)
		load_engine_assets();

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

	/* get all shaders as mutable reference */
	umap<string, shader_asset>& content_manager::touch_shaders()
	{
		return m_shaders;
	}

	/* loads all engine assets (/engine/assets/...) */
	void content_manager::load_engine_assets()
	{
		const auto engine_assets_dir = get_engine_directory(engine_directory::assets);
		load_assets(e_asset_origin::engine, engine_assets_dir);
	}

	/* loads all game assets (/game_name/assets/...) */
	void content_manager::load_game_assets(const string& game_name)
	{
		const auto game_assets_dir = get_game_directory(game_name, game_directory::assets);
		load_assets(e_asset_origin::game, game_assets_dir);
	}

	/* loads a single asset file at path into the content manager */
	result<> content_manager::load_file(const string& path_str)
	{
		using result_type = result<>;

		if (!path::exists(path_str))
			return result_type::make_error("file at path not found!");

		const path as_path = path(path_str);

		const bool without_extension = false;
		const string& filename = to_string(as_path.get_filename(without_extension));

		auto asset_type_res = get_asset_type_from_extension(to_string(as_path.get_extension()));
		if (asset_type_res.is_unex())
		{
			return result_type::make_error("failed finding asset_type from file's extension!");
		}

		switch (asset_type_res.get())
		{
		default:
		case e_asset_type::count:
			return result_type::make_error("deducting image load args for this file extension not supported yet...");

		case e_asset_type::scene:
			m_scenes[filename].load(path_str, k_default_scene_import_args);
			return {};
			break;
		}

		return result_type::make_error("failed loading file...");
	}

	/* given an origin (category), load all assets in that category */
	void content_manager::load_assets(e_asset_origin origin, const path& root)
	{
		const string& root_path_str = to_string(root.get_full_path());
		const vector<path> fbx_files = path::get_files_in_directory(root_path_str, true, ".fbx").get();
		const vector<path> obj_files = path::get_files_in_directory(root_path_str, true, ".obj").get();
		const vector<path> png_files = path::get_files_in_directory(root_path_str, true, ".png").get();
		const vector<path> hlsl_files = path::get_files_in_directory(root_path_str, true, ".hlsl").get();

		// load cubemap (hack)
		{
			imp::cubemap_load_args args{};
			stat_array<string, 6u> cubemap_side_files{};
			uint32 i = 0u;
			for (const path& png : png_files)
			{
				const bool without_extension = false;
				const string& filename = to_string(png.get_filename(without_extension));
				const string& full_path = to_string(png.get_full_path());

				if (str::contains(filename, "graycloud"))
				{
					cubemap_side_files[i++] = full_path;
				}
			}

			cubemap_asset& item = m_cubemaps["graycloud"];
			args.m_hacky_paths = &cubemap_side_files;
			item.load(cubemap_side_files[0], args);
		}
		
		// load fbxs & objs
		async::dispatch_for<path>(obj_files, [this](const path& file)
		{
			imp::scene_load_args args{};
			args.m_bake_transforms = true;
			args.m_pre_scale = 1;
			const bool without_extension = false;
			scene_asset& item = m_scenes[to_string(file.get_filename(without_extension))];
			item.load(file, args);
		});
		async::dispatch_for<path>(fbx_files, [this](const path& file)
		{
			imp::scene_load_args args{};
			args.m_bake_transforms = true;
			args.m_pre_scale = 1;
			const bool without_extension = false;
			scene_asset& item = m_scenes[to_string(file.get_filename(without_extension))];
			item.load(file, args);
		});

		// load hlsls
		static shader::compile_args compile_args{};
		if (origin == e_asset_origin::engine)
		{
			compile_args.m_include_folder = to_string(root.get_full_path()) + "/engine/shaders/";
		}
		
		compile_args.m_signature.m_target = shader::e_shader_target::_6_6;
		compile_args.m_reflection_enabled = true;
		compile_args.m_defines = {};
#if INFLUX_DEBUG
		compile_args.set_debug_level(true);
		compile_args.m_pbd_enabled = true;
		compile_args.m_pdb_folder = to_string(get_engine_directory(engine_directory::shaderpdb).get_full_path()).c_str();
#else
		compile_args.m_compile_debug = false;
		compile_args.m_pbd = false;
#endif
		async::dispatch_for<path>(hlsl_files, [this, root](const path& file)
		{
			const bool without_extension = false;
			const string& filename = to_string(file.get_filename(without_extension));
			const string& full_path = to_string(file.get_full_path());

#if INFLUX_DEBUG
			compile_args.m_pdb_filename = filename;
#endif
			compile_args.m_signature.m_filename = filename;

			const string& file_content = path::read_all_to_string(full_path).get();
			if (str::contains(file_content, "[shader(\"vertex\")]", false))
			{
				shader_asset& vs_item = m_shaders[filename + "_vs"];
				compile_args.m_signature.m_type = shader::e_shader_type::vs;
				compile_args.m_signature.m_entrypoint = "main_vs";
				compile_args.m_signature.cache_id();
				vs_item.load(file, { compile_args });
			}

			if (str::contains(file_content, "[shader(\"pixel\")]", false))
			{
				shader_asset& ps_item = m_shaders[filename + "_ps"];
				compile_args.m_signature.m_type = shader::e_shader_type::ps;
				compile_args.m_signature.m_entrypoint = "main_ps";
				compile_args.m_signature.cache_id();
				ps_item.load(file, { compile_args });
			}

			if (str::contains(file_content, "[shader(\"compute\")]", false))
			{
				shader_asset& cs_item = m_shaders[filename + "_cs"];
				compile_args.m_signature.m_type = shader::e_shader_type::cs;
				compile_args.m_signature.m_entrypoint = "main_cs";
				compile_args.m_signature.cache_id();
				cs_item.load(file, { compile_args });
			}
		});

		// load pngs
		async::dispatch_for<path>(png_files, [this](const path& file)
		{
			const bool without_extension = false;
			const string& filename = to_string(file.get_filename(without_extension));

			imp::image_load_args args{};
			image_asset& item = m_images[filename];
			item.load(file, args);
		});
	}
}