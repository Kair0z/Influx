#include "engine_pch.h"
#include "content_manager.h"
// influx::core
#include "core/log.h"
#include "core/time.h"
#include "core/file.h"
// influx::engine
#include "engine_files.h"
#include "editor/editor_manager.h"
#include "rendering/render_manager.h"
// influx::async
#include "influx_async.h"
// influx::import
#include "influx_import.h"
// influx::shader
#include "influx_shader.h"
// imgui
#include "imgui.h"

#define USE_ASYNC_LOADING 0

namespace influx::engine
{
	using namespace assets;

	template <typename _t, typename _func>
	inline result<> dispatch_for(const vector<_t>& vector, _func&& func)
	{
#if USE_ASYNC_LOADING
		auto dispatch_res = async::dispatch_for<_t>(vector, func);
		if (dispatch_res.is_success() == false)
			return result<>::make_error("async error!");
		
		return {};
#else
		for (uint64 i = 0u; i < vector.size(); ++i) {
			func(vector[i]);
		}
		return {};
#endif
	}

	class content_ui final : public editor::editor_window
	{
	public:
		virtual void on_run() override
		{
			static asset_manager& content = get_engine()->get_assetman();
			static render_manager& renderer = get_engine()->get_renderer();

			set_name("content");

			if (ImGui::BeginTabBar("content"))
			{
				if (ImGui::BeginTabItem("scenes"))
				{
					// "scene:filepath"
					for (const auto& pair : content.get_scenes())
					{
						const string& name = pair.first;
						const scene_asset& scene_asset = pair.second;

						if (scene_asset.is_loaded())
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
							if (pair.second.is_loaded())
							{
								ImGui::TableNextColumn();

								const string& name = pair.first;
								const image_asset& image = pair.second;
								const math::vectoru2& image_dims = image.m_resource.m_dimensions;
								ImGui::TextWrapped("%s", name.c_str());

								auto get_texture_res = renderer.get_texture2D(name);
								if (get_texture_res.is_success() && get_texture_res.get() != nullptr)
									ImGui::Image( reinterpret_cast<ImTextureID>(get_texture_res.get()), { 24u, 24u });
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
						if (pair.second.is_loaded())
							ImGui::Text("shader:%ull - ms:%f", pair.first, pair.second.get_load_ms());
					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}
		}
	};

	static const load_args<e_asset_type::shader> k_default_shader_import_args
	{{
		
	}};

	static const load_args<e_asset_type::scene> k_default_scene_import_args
	{
		.m_pre_scale = 1.0f,
		.m_bake_transforms = false
	};

	static const load_args<e_asset_type::image> k_default_image_import_args
	{
		
	};

	asset_manager::asset_manager()
	{
		// immediately start kicking engine loading (may as well)
		// load_engine_assets();
		editor::editor_manager::static_window<content_ui>("content");

		const path engine_assets_dir = get_engine_directory(engine_directory::assets);
		load_file(engine_assets_dir.get_full_path() + "/meshes/cafeleblanc.fbx");
	}

	asset_manager::~asset_manager()
	{
	}

	/* loads all engine assets (/engine/assets/...) */
	void asset_manager::load_engine_assets()
	{
		const auto engine_assets_dir = get_engine_directory(engine_directory::assets);
		load_assets(e_asset_origin::engine, engine_assets_dir);
	}

	/* loads all game assets (/game_name/assets/...) */
	void asset_manager::load_game_assets(const string& game_name)
	{
		const auto project_assets_dir = get_project_directory(game_name, game_directory::assets);
		load_assets(e_asset_origin::project, project_assets_dir);
	}

	result<> asset_manager::load_fbx(const path& filepath)
	{
		return load_scene(filepath);
	}

	result<> asset_manager::load_obj(const path& filepath)
	{
		return load_scene(filepath);
	}

	result<> asset_manager::load_scene(const path& filepath)
	{
		using result_type = result<>;
		const bool without_extension = false;
		const string filename = to_string(filepath.get_filename(without_extension));

		imp::scene_load_args args{};
		args.m_bake_transforms = true;
		args.m_pre_scale = 1;

		const scene_id scene_id = make_scene_id(filename);
		scene_asset& item = m_scenes[scene_id];

		auto load_res = item.load(filepath, args);
		if (load_res.is_fail())
			return result_type::make_error("load scene failed!");

		// get the imp::scene from the loaded scene, and load each separate mesh as an asset
		cptr<scene_data> loaded_scene = load_res.get();
		const imp::scene_data& imp_scene = loaded_scene->m_imported_data;
		for (const imp::scene_data::mesh& mesh : imp_scene.get_meshes())
		{
			const string mesh_name = imp_scene.get_name(mesh);
			mesh_id mesh_id = make_mesh_id(mesh_name);

			mesh_data mesh_data{};
			mesh_data.m_imported_data = mesh;
			load_mesh(mesh_id, mesh_data).get();
		}
	}

	result<> asset_manager::load_mesh(const mesh_id& id, const mesh_data& data)
	{
		using result_type = result<>;
		mesh_asset& item = m_meshes[id];
		item.load()
	}

	/* loads a single asset file at path into the content manager */
	result<> asset_manager::load_file(const string& path_str)
	{
		using result_type = result<>;

		if (!path::exists(path_str))
			return result_type::make_error("path_str is an invalid path!");

		// query the asset type
		const path as_path = path(path_str);
		auto asset_type_res = get_asset_type_from_extension(to_string(as_path.get_extension()));
		if (asset_type_res.is_fail())
		{
			return result_type::make_error("failed finding asset_type from file's extension!");
		}

		// handle the asset type
		switch (asset_type_res.get())
		{
		default:
		case e_asset_type::scene:
			load_scene(as_path).get();
			return {};
		}

		return result_type::make_error("failed loading file...");
	}

	/* given an origin (category), load all assets in that category */
	void asset_manager::load_assets(e_asset_origin origin, const path& root)
	{
		const string& root_path_str = to_string(root.get_full_path());
		const vector<path> fbx_files = path::get_files_in_directory(root_path_str, true, ".fbx").get();
		const vector<path> obj_files = path::get_files_in_directory(root_path_str, true, ".obj").get();
		const vector<path> png_files = path::get_files_in_directory(root_path_str, true, ".png").get();
		
#if 0
		// load fbxs & objs
		dispatch_for<path>(obj_files, [this](const path& file) { load_scene(file).get(); });
		dispatch_for<path>(fbx_files, [this](const path& file) { load_scene(file).get(); });
#endif

		// load_shaders(origin, root);

		// load pngs
		dispatch_for<path>(png_files, [this](const path& file)
		{
			const bool without_extension = false;
			const string& filename = to_string(file.get_filename(without_extension));

			const image_id img_id = make_image_id(filename);
			imp::image_load_args args{};
			image_asset& item = m_images[img_id];
			item.load(file, args, true);
		});
	}

	void asset_manager::load_shaders(e_asset_origin origin, const path& root)
	{
		const string& root_path_str = to_string(root.get_full_path());
		const vector<path> hlsl_files = path::get_files_in_directory(root_path_str, true, ".hlsl").get();

		// global compile args
		static shader::compile_args compile_args{};
		if (origin == e_asset_origin::engine)
		{
			compile_args.m_include_folder = to_string(root.get_full_path()) + "/engine/shaders/";
		}
		compile_args.m_target = shader::e_shader_target::_6_6;
		compile_args.m_reflection_enabled = true;
		compile_args.m_defines = {};
#if INFLUX_DEBUG
		compile_args.set_debug_level(true);
		compile_args.set_pdb_enabled(true);
		compile_args.m_pdb_folder = to_string(get_engine_directory(engine_directory::shaderpdb).get_full_path()).c_str();
#else
		compile_args.set_debug_level(false);
		compile_args.set_pdb_enabled(false);
#endif

		dispatch_for<path>(hlsl_files, [this, root](const path& file)
		{
			const auto parse_result = shader::parse_shaders_in_file(to_string(file.get_full_path()));
			if (!parse_result.is_success()) 
				return;

			// set the individual compile args for each parse result
			const bool without_extension = false;
			const string& filename = to_string(file.get_filename(without_extension));
			compile_args.m_pdb_filename = filename;

			imp::shader_load_args load_args{};
			load_args.m_compile_args = compile_args;

			// for each parsed shader in shadermap...
			for (const auto& pair : parse_result.get().m_shadermap)
				for (const auto& shader : pair.second)
				{
					const shader::e_shader_type type = pair.first;
					const shader::shader_signature parsed_signature = shader.m_signature;
					const string& parsed_tag = parsed_signature.get_tag();

					const shader_id sh_id = make_shader_id(parsed_tag);
					shader_asset& cs_item = m_shaders[sh_id];
					cs_item.load(file, load_args, true);
				}
		});
	}
}