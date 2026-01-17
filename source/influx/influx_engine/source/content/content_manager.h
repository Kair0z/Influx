#pragma once

// influx::engine
#include "content/asset.h"
namespace influx::engine
{
	class engine;
}

// influx::imp
#include "influx_import.h"

namespace influx::engine
{
	class asset_manager final
	{
		template <assets::e_asset_type _e>
		using asset_map = umap<assets::data_id<_e>, assets::asset_item<_e>>;

		asset_map<assets::e_asset_type::scene> m_scenes;
		asset_map<assets::e_asset_type::mesh> m_meshes;
		asset_map<assets::e_asset_type::cubemap> m_cubemaps;
		asset_map<assets::e_asset_type::image> m_images;
		asset_map<assets::e_asset_type::shader> m_shaders;
		
	public:
		asset_manager();
		~asset_manager();

		template <assets::e_asset_type _e>
		const asset_map<_e>& get_asset_map() const
		{
			if constexpr (_e == assets::e_asset_type::scene) return m_scenes;
			else if constexpr (_e == assets::e_asset_type::mesh) return m_meshes;
			else if constexpr (_e == assets::e_asset_type::cubemap) return m_cubemaps;
			else if constexpr (_e == assets::e_asset_type::image) return m_images;
			else if constexpr (_e == assets::e_asset_type::shader) return m_shaders;
		}

		template <assets::e_asset_type _e>
		inline assets::asset_item<_e> const* find_asset(const assets::data_id<_e>& id) const
		{
			const asset_map<_e>& the_map = get_asset_map<_e>();
			if (the_map.contains(id))
				return { &the_map.at(id) };
			else
				return nullptr;
		}

		/* loads a single asset file at path into the content manager */
		result<> load_file(const string& path);

		/* loads all engine assets (/engine/assets/...) */
		void load_engine_assets();

		/* loads all game assets (/game_name/assets/...) */
		void load_game_assets(const string& game_name);
		
		result<> load_fbx(const path& filepath);
		result<> load_obj(const path& filepath);
		result<> load_scene(const path& filepath);
		result<> load_mesh(const mesh_id& id, const mesh_data& data);

		/* finds a loaded mesh given a mesh_name*/
		inline result<imp::scene_data::mesh*> find_mesh(const string& mesh_name)
		{
			using result_type = result<imp::scene_data::mesh*>;

			/* split the mesh_name into parts */
			const vector<string>& parts = mesh_name.split("_");
			const string scene_name = parts.size() > 0u ? parts[0u] : "";
			const string index_str = parts.size() > 1u ? parts[1u] : "";
			const uint32 mesh_idx = !index_str.empty() ? std::stoi(index_str) : 0u;

			return find_mesh(scene_name, mesh_idx);
		}

		/* finds a loaded mesh given a scene_name & index */
		inline result<imp::scene_data::mesh*> find_mesh(const string& scene_name, uint32 mesh_idx)
		{
			using result_type = result<imp::scene_data::mesh*>;

			const assets::scene_id id = assets::make_scene_id(scene_name);
			if (m_scenes.contains(id))
			{
				return &m_scenes[id].m_resource.get_mesh(mesh_idx);
			}

			return result_type::make_error("could not find scene at scene_name, so a mesh was not found");
		}
		
		/* given a scene_asset (fbx) and an index, returns the mesh name at that index */
		static result<string> get_scene_mesh_name(const assets::scene_asset& item, const uint32 idx)
		{
			using result_type = result<string>;
			return item.m_name + "_" + to_string(idx);
		}

		umap<assets::scene_id, assets::scene_asset> get_scenes() const {
			return get_asset_map<assets::e_asset_type::scene>();
		}
		umap<assets::mesh_id, assets::mesh_asset> get_meshes() const {
			return get_asset_map<assets::e_asset_type::mesh>();
		}
		umap<assets::shader_id, assets::shader_asset> get_shaders() const {
			return get_asset_map<assets::e_asset_type::shader>();
		}
		umap<assets::cubemap_id, assets::cubemap_asset> get_cubemaps() const {
			return get_asset_map<assets::e_asset_type::cubemap>();
		}
		umap<assets::image_id, assets::image_asset> get_images() const {
			return get_asset_map<assets::e_asset_type::image>();
		}

	private:
		/* given an origin (category), load all assets in that category */
		void load_assets(assets::e_asset_origin origin, const path& root);
		void load_shaders(assets::e_asset_origin origin, const path& root);
	};
}