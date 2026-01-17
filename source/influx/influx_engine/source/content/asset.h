#pragma once

#include "core/string.h"

// influx::engine
#include "engine_files.h"

// influx::import
#include "influx_import.h"
#if 0 
namespace influx::imp
{
	struct scene_load_args;
	struct image_load_args;
	struct cubemap_load_args;
	struct shader_load_args;
	struct shader_data;
	struct scene_data;
	struct image_data;
	struct cubemap_data;
	struct shader_data;
}
#endif

namespace influx::engine::assets
{
	using asset_id = uint64;
	using scene_id = asset_id;
	using mesh_id = asset_id;
	using image_id = asset_id;
	using cubemap_id = asset_id;
	using shader_id = asset_id;
	static constexpr asset_id k_invalid_id = (asset_id)-1;

	static const asset_id make_asset_id(const string& name) {
		hash<string> hasher; return hasher(name);
	}
	static const scene_id make_scene_id(const string& name) { return make_asset_id(name); }
	static const scene_id make_image_id(const string& name) { return make_asset_id(name); }
	static const scene_id make_mesh_id(const string& name) { return make_asset_id(name); }
	static const scene_id make_cubemap_id(const string& name) { return make_asset_id(name); }
	static const scene_id make_shader_id(const string& name) { return make_asset_id(name); }

	enum class e_asset_type : uint8
	{
		scene,		// fbx files
		mesh,		// fbx files
		image,		// png files
		cubemap,	
		shader,		// hlsl files
		count
	};

	static const result<e_asset_type> get_asset_type_from_extension(const string& extension)
	{
		using result_type = result<e_asset_type>;
		if (extension == ".fbx" || extension == ".obj") return e_asset_type::scene;
		if (extension == ".png")						return e_asset_type::image;
		if (extension == ".hlsl")						return e_asset_type::shader;
		return result_type::make_error("asset_type not deducable for this extension!");
	}
	
	enum class e_asset_origin : uint8
	{
		unknown,
		engine,
		project,
		imported,
		count
	};
	enum class e_asset_load_state : uint8
	{
		unloaded,
		loading,
		loaded,
		failed_load,
		count
	};

	using scene_load_args = imp::scene_load_args;
	using mesh_load_args = scene_load_args;
	using image_load_args = imp::image_load_args;
	using cubemap_load_args = imp::cubemap_load_args;
	using shader_load_args = imp::shader_load_args;

	struct scene_data final
	{
		imp::scene_data m_imported_data;
	};
	struct mesh_data final
	{
		imp::mesh_data m_imported_data;
	};
	struct image_data final
	{
		imp::image_data m_imported_data;
	};
	struct cubemap_data final
	{
		image_id m_images[6u];
	};
	struct shader_data final
	{
		vector<imp::shader_data> m_imported_data;
	};

	result<scene_data> load_scene_data(const string& path, const imp::scene_load_args& args);
	result<mesh_data> load_mesh_data(const string& path, const imp::scene_load_args& args);
	result<image_data> load_image_data(const string& path, const imp::image_load_args& args);
	result<cubemap_data> load_cubemap_data(const string& path, const imp::cubemap_load_args& args);
	result<shader_data> load_shader_data(const string& path, const imp::shader_load_args& args);

	/* type of id */
	template <e_asset_type _t>
	using data_id = std::tuple_element_t < static_cast<uint64>(_t), std::tuple<
		scene_id,
		mesh_id,
		image_id,
		cubemap_id,
		shader_id>>;

	/* type of data obtained after loading */
	template <e_asset_type _t>
	using data_type = std::tuple_element_t<static_cast<uint64>(_t), std::tuple<
		scene_data,
		mesh_data,
		image_data,
		cubemap_data,
		shader_data>>;

	/* type of arguments for loading */
	template <e_asset_type _t>
	using load_args = std::tuple_element_t<static_cast<uint64>(_t), std::tuple<
		scene_load_args,
		mesh_load_args,
		image_load_args,
		cubemap_load_args,
		shader_load_args>>;

	template <e_asset_type _t>
	struct asset_item final
	{
		using load_args = load_args<_t>;
		using data_type = data_type<_t>;
		using id_type = data_id<_t>;
		
		id_type m_id = k_invalid_id;

		/* arguments by which this asset was last loaded */
		load_args m_last_args{};

		/* data of the asset loaded */
		data_type m_resource{};

		e_asset_load_state m_state	= e_asset_load_state::unloaded;
		e_asset_origin m_origin = e_asset_origin::unknown;

		string m_name = "";
		string m_path = "";
		string m_extension = "";
		
		time::point m_time_loadstart{};
		time::point m_time_loadend{};

		// load from filepath
		inline result<cptr<data_type>> load(const path& path, const load_args& args, bool allow_reload = false)
		{
			using result_type = result<cptr<data_type>>;
			const e_asset_load_state current_load_state = get_loadstate();

			if (!path.exists())
				return result_type::make_error("file at path doesn't exist!");

			if (current_load_state == e_asset_load_state::loading)
				return result_type::make_error("this asset is already currently loading. Skipping load.");

			if (current_load_state == e_asset_load_state::loaded && allow_reload == false)
				return result_type::make_error("this asset is already loaded, and allow_reload is false. Skipping load.");

			// store path-based string data
			const bool without_extension = false;
			m_name = to_string(path.get_filename(without_extension));
			m_path = to_string(path.get_full_path());
			m_extension = to_string(path.get_extension());
			m_id = make_asset_id(m_name);

			// do the load
			bool is_load_success = false;
			m_time_loadstart = time::get_now();
			set_loadstate(e_asset_load_state::loading);
			if constexpr (_t == e_asset_type::scene)
			{
				if (auto res = load_scene_data(m_path, args))
				{
					m_resource = res.get();
					is_load_success = true;
				}
			}
			else if constexpr (_t == e_asset_type::image)
			{
				if (auto res = load_image_data(m_path, args))
				{
					m_resource = res.get();
					is_load_success = true;
				}
			}
			else if constexpr (_t == e_asset_type::cubemap)
			{
				if (auto res = load_cubemap_data(m_path, args))
				{
					m_resource = res.get();
					is_load_success = true;
				}
			}
			else if constexpr (_t == e_asset_type::shader)
			{
				if (auto res = load_shader_data(m_path, args))
				{
					m_resource = res.get();
					is_load_success = true;
				}
			}
			m_time_loadend = time::get_now();
			m_last_args = args;

			// post-load
			// on success, create a mirror flx file
			if (is_load_success)
			{
				set_loadstate(e_asset_load_state::loaded);
				create_flx_file();
				return {};
			}
			else
			{
				set_loadstate(e_asset_load_state::failed_load);
				return result_type::make_error("file at path failed to load as a valid resource.");
			}
			return {};
		}
		
		// load from memory
		inline result<cptr<data_type>> load(const data_type& data, bool allow_reload = false)
		{
			using result_type = result<cptr<data_type>>;
			if (current_load_state == e_asset_load_state::loading)
				return result_type::make_error("this asset is already currently loading. Skipping load.");

			if (current_load_state == e_asset_load_state::loaded && allow_reload == false)
				return result_type::make_error("this asset is already loaded, and allow_reload is false. Skipping load.");


		}
		inline result<cptr<data_type>> reload(const load_args& args)
		{
			return load(m_path, args, true);
		}

		inline result<cptr<data_type>> reload()
		{
			return reload(m_last_args);
		}

		inline bool is_loaded() const
		{
			return m_state == e_asset_load_state::loaded;
		}

		inline e_asset_origin get_origin() const
		{
			return m_origin;
		}

		inline bool is_engine() const
		{
			return m_origin == e_asset_origin::engine;
		}

		inline bool is_game() const
		{
			return m_origin == e_asset_origin::game;
		}

		inline e_asset_load_state get_loadstate() const
		{
			return m_state;
		}

		inline asset_id get_id() const
		{
			return m_id;
		}

		inline const data_type& get_resource() const
		{
			return m_resource;
		}

		inline float get_load_ms() const
		{
			return time::get_ms_between<float>(m_time_loadend, m_time_loadstart);
		}

	private:
		inline void set_loadstate(e_asset_load_state new_state)
		{
			if (new_state == e_asset_load_state::loading)
			{
				m_time_loadstart = time::get_now();
			}

			if (new_state == e_asset_load_state::loaded)
			{
				m_time_loadend = time::get_now();
			}

			m_state = new_state;
		}

		inline void create_flx_file()
		{
			const string& og_path = m_path;
			const string og_friendly = get_friendly_name(og_path);

			const string old_extension = m_extension;
			const string new_extension = ".flx";

			const string prestring = "/assets/";
			string relative = og_friendly.substr(prestring.size());
			relative = relative.substr(0u, relative.size() - old_extension.size()) + new_extension;

			const string generated_assets = to_string(get_engine_directory(engine_directory::assets_gen).get_full_path());
			const string new_path = generated_assets + relative;

			path::create_file(new_path);
		}
	};

	using scene_asset = asset_item<e_asset_type::scene>;
	using mesh_asset = asset_item<e_asset_type::mesh>;
	using image_asset = asset_item<e_asset_type::image>;
	using shader_asset = asset_item<e_asset_type::shader>;
	using cubemap_asset = asset_item<e_asset_type::cubemap>;
}