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
	// content in influx::engine
	class content_manager final
	{
		umap<string, scene_asset> m_scenes;
		umap<string, image_asset> m_images;
		umap<string, shader_asset> m_shaders;
		umap<string, cubemap_asset> m_cubemaps;
		thread m_loading_thread;
		time::point m_start_engine_resources;

	public:
		content_manager(engine* engine);
		~content_manager();

		const umap<string, scene_asset>& get_scenes() const;
		const umap<string, image_asset>& get_images() const;
		const umap<string, shader_asset>& get_shaders() const;
		const umap<string, cubemap_asset>& get_cubemaps() const;
		umap<string, shader_asset>& touch_shaders();
		
		template <typename _t>
		inline _t const* find(const string& asset_name) const
		{
			if constexpr (std::is_same_v<_t, scene_asset>)
			{
				if (get_scenes().contains(asset_name))
					return { &get_scenes().at(asset_name) };
			}
			else if constexpr (std::is_same_v<_t, image_asset>)
			{
				if (get_images().contains(asset_name))
					return { &get_images().at(asset_name) };
			}
			else if constexpr (std::is_same_v<_t, shader_asset>)
			{
				if (get_shaders().contains(asset_name))
					return { &get_shaders().at(asset_name) };
			}
			else if constexpr (std::is_same_v<_t, cubemap_asset>)
			{
				if (get_cubemaps().contains(asset_name))
					return { &get_cubemaps().at(asset_name) };
			}
			
			return nullptr;
		}

		void load(const string& path);
		void load_engine_assets(engine* engine);
		void load_game_assets(const string& game_name, engine* engine);
		
		// finding individual meshes
		inline imp::scene_data::mesh* find_mesh(const string& mesh_name)
		{
			const vector<string>& parts = str::split(mesh_name, "_");
			const string scene_name = parts.size() > 0u ? parts[0u] : "";
			const string index_str = parts.size() > 1u ? parts[1u] : "";
			const uint32 mesh_idx = !index_str.empty() ? std::stoi(index_str) : 0u;

			return find_mesh(scene_name, mesh_idx);
		}
		inline imp::scene_data::mesh* find_mesh(const string& scene_name, uint32 mesh_idx)
		{
			if (m_scenes.contains(scene_name))
			{
				return &m_scenes[scene_name].m_resource.get_mesh(mesh_idx);
			}

			return nullptr;
		}
		static string get_scene_mesh_name(const scene_asset& item, const uint32 idx)
		{
			return item.m_name + "_" + to_string(idx);
		}

		// update externally changed files
		void update_filechanges();

	private:
		void load_assets(engine* engine, e_asset_origin, const file& root);
	};
}