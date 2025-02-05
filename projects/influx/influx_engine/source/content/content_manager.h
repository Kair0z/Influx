#pragma once

// influx::core
#include "core/container/map.h"
#include "core/container/vector.h"
#include "core/time.h"
#include "core/threading/thread.h"

// influx::import
#include "influx_import.h"

// influx::async
#include "influx_async.h"

// influx::engine
namespace influx::engine
{
	class engine;
}

namespace influx::engine
{
	// content in influx::engine
	// differentiates between 2 categories.
	// - resources: the raw input items that the engine accepts and can convert into assets
	// - assets: influx-native representations of resources (.flx)
	class content_manager final
	{
		enum class e_asset_type : uint8
		{
			scene,
			image,
			shader,
			count
		};

		template <e_asset_type _t>
		using data_type = std::tuple_element_t<static_cast<uint64>(_t), std::tuple<
			imp::scene_data,
			imp::image_data,
			imp::shader_data>>;

		template <e_asset_type _t>
		using load_args = std::tuple_element_t<static_cast<uint64>(_t), std::tuple<
			imp::scene_load_args,
			imp::image_load_args,
			shader::compile_args>>;

		enum class e_asset_origin : uint8
		{
			engine,
			game,
			count
		};

		enum class e_load_state : uint8
		{
			unloaded,
			loading,
			loaded,
			count
		};

		template <e_asset_type _t>
		struct asset_item final
		{
			using data_type = data_type<_t>;

			asset_item()
			{
				set_loadstate(e_load_state::unloaded);
			}

			void load(const string& path, const load_args<_t>& args, bool reload = false)
			{
				// use reload instead
				if (get_loadstate() == e_load_state::count) return;
				if (get_loadstate() == e_load_state::loading) return;
				if (get_loadstate() == e_load_state::loaded && reload == false) return;

				m_path = path;
				m_time_loadstart = time::get_now();
				set_loadstate(e_load_state::loading);
				if constexpr (_t == e_asset_type::scene)
				{
					m_resource = load_scene_data(m_path, args);
				}
				else if constexpr (_t == e_asset_type::image)
				{
					m_resource = load_image_data(m_path, args);
				}
				else if constexpr (_t == e_asset_type::shader)
				{
					m_resource = load_shader_data(m_path, args);
				}
				set_loadstate(e_load_state::loaded);
				m_time_loadend = time::get_now();
				m_last_args = args;
			}

			void reload(const load_args<_t>& args)
			{
				load(m_path, args, true);
			}

			void reload()
			{
				reload(m_last_args);
			}

			bool is_loaded() const
			{
				return m_state == e_load_state::loaded;
			}

			e_asset_origin get_origin() const
			{
				return m_origin;
			}

			bool is_engine() const
			{
				return m_origin == e_asset_origin::engine;
			}

			bool is_game() const
			{
				return m_origin == e_asset_origin::game;
			}

			e_load_state get_loadstate() const
			{
				return m_state;
			}

			const data_type& get_resource() const
			{
				return m_resource;
			}

			void set_loadstate(e_load_state new_state)
			{
				if (new_state == e_load_state::loading)
				{
					m_time_loadstart = time::get_now();
				}

				if (new_state == e_load_state::loaded)
				{
					m_time_loadend = time::get_now();
				}

				m_state = new_state;
			}

			float get_load_ms() const
			{
				return time::get_ms_between<float>(m_time_loadend, m_time_loadstart);
			}

			data_type m_resource{}; // the raw resource
			e_load_state m_state{};
			e_asset_origin m_origin{};
			string m_name;
			string m_path;
			load_args<_t> m_last_args{};

			time::point m_time_loadstart{};
			time::point m_time_loadend{};
		};

		static imp::scene_data load_scene_data(const string& path, const imp::scene_load_args& args);
		static imp::image_data load_image_data(const string& path, const imp::image_load_args& args);
		static imp::shader_data load_shader_data(const string& path, const shader::compile_args& args);

	public:
		using scene_item = asset_item< e_asset_type::scene >;
		using image_item = asset_item< e_asset_type::image >;
		using shader_item = asset_item< e_asset_type::shader >;

		content_manager(engine* engine);
		~content_manager();

		const umap<string, scene_item>& get_scenes() const;
		const umap<string, image_item>& get_images() const;
		const umap<string, shader_item>& get_shaders() const;
		umap<string, shader_item>& touch_shaders();
		
		template <typename _t>
		_t const* find(const string& asset_name) const
		{
			if constexpr (std::is_same_v<_t, scene_item>) 
			{
				if (get_scenes().contains(asset_name))
					return { &get_scenes().at(asset_name) };
			}
			else if constexpr (std::is_same_v<_t, image_item>) 
			{
				if (get_images().contains(asset_name))
					return { &get_images().at(asset_name) };
			}
			else if constexpr (std::is_same_v<_t, shader_item>) 
			{
				if (get_shaders().contains(asset_name))
					return { &get_shaders().at(asset_name) };
			}
			
			return nullptr;
		}

		// loads /influx/assets/
		void load_engine_assets(engine* engine);

		// loads /influx/games/'game_name'/assets/
		void load_game_assets(const string& game_name, engine* engine);

		// finding meshes
		imp::scene_data::mesh* find_mesh(const string& mesh_name)
		{
			const vector<string>& parts = str::split(mesh_name, "_");
			const string scene_name = parts.size() > 0u ? parts[0u] : "";
			const string index_str = parts.size() > 1u ? parts[1u] : "";
			const uint32 mesh_idx = !index_str.empty() ? std::stoi(index_str) : 0u;

			return find_mesh(scene_name, mesh_idx);
		}
		imp::scene_data::mesh* find_mesh(const string& scene_name, uint32 mesh_idx)
		{
			if (m_scenes.contains(scene_name))
			{
				return &m_scenes[scene_name].m_resource.get_mesh(mesh_idx);
			}

			return nullptr;
		}
		static string get_scene_mesh_name(const scene_item& item, const uint32 idx)
		{
			return item.m_name + "_" + to_string(idx);
		}

	private:
		umap<string, scene_item> m_scenes;
		umap<string, image_item> m_images;
		umap<string, shader_item> m_shaders;

		thread m_loading_thread;

		time::point m_start_engine_resources;

		void load_assets(engine* engine, e_asset_origin, const file& root);
	};

	using image_asset = content_manager::image_item;
	using scene_asset = content_manager::scene_item;
	using shader_asset = content_manager::shader_item;
}