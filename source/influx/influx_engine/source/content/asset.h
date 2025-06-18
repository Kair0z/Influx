#pragma once

#include "core/string.h"

// influx::engine
#include "file/engine_files.h"

// influx::import
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

namespace influx::engine
{
	enum class e_asset_type : uint8
	{
		scene,		// fbx files
		image,		// png files
		cubemap,	
		shader,		// hlsl files
		count
	};

	static result<e_asset_type> get_asset_type_from_extension(const string& extension)
	{
		using result_type = result<e_asset_type>;

		if (extension == ".fbx") return e_asset_type::scene;
		if (extension == ".png") return e_asset_type::image;
		if (extension == ".hlsl") return e_asset_type::shader;

		return result_type::make_error("asset_type not deducable for this extension!");
	}

	enum class e_asset_origin : uint8
	{
		unknown,
		engine,
		game,
		imported,
		count
	};

	enum class e_load_state : uint8
	{
		unloaded,
		loading,
		loaded,
		failed_load,
		count
	};

	result<imp::scene_data> load_scene_data(const string& path, const imp::scene_load_args& args);
	result<imp::image_data> load_image_data(const string& path, const imp::image_load_args& args);
	result<imp::cubemap_data> load_cubemap_data(const string& path, const imp::cubemap_load_args& args);
	result<imp::shader_data> load_shader_data(const string& path, const imp::shader_load_args& args);

	/* type of data obtained after loading */
	template <e_asset_type _t>
	using data_type = std::tuple_element_t<static_cast<uint64>(_t), std::tuple<
		imp::scene_data,
		imp::image_data,
		imp::cubemap_data,
		imp::shader_data>>;

	/* type of arguments for loading */
	template <e_asset_type _t>
	using load_args = std::tuple_element_t<static_cast<uint64>(_t), std::tuple<
		imp::scene_load_args,
		imp::image_load_args,
		imp::cubemap_load_args,
		imp::shader_load_args>>;

	template <e_asset_type _t>
	struct asset_item final
	{
		using load_args = load_args<_t>;
		using data_type = data_type<_t>;

		/* arguments by which this asset was last loaded */
		load_args m_last_args{};

		/* data of the asset loaded */
		data_type m_resource{};

		e_load_state m_state	= e_load_state::unloaded;
		e_asset_origin m_origin = e_asset_origin::unknown;

		string m_name = "";
		string m_path = "";
		string m_extension = "";
		
		time::point m_time_loadstart{};
		time::point m_time_loadend{};

		inline result<> load(const path& path, const load_args& args, bool allow_reload = false)
		{
			using result_type = result<>;

			const e_load_state current_load_state = get_loadstate();

			if (!path.exists())
				return result_type::make_error("file at path doesn't exist!");

			if (current_load_state == e_load_state::loading)
				return result_type::make_error("this asset is already currently loading. Skipping load.");

			if (current_load_state == e_load_state::loaded && allow_reload == false)
				return result_type::make_error("this asset is already loaded, and allow_reload is false. Skipping load.");

			// store path-based string data
			const bool without_extension = false;
			m_name = to_string(path.get_filename(without_extension));
			m_path = to_string(path.get_full_path());
			m_extension = to_string(path.get_extension());

			// do the load
			bool is_load_success = false;
			m_time_loadstart = time::get_now();
			set_loadstate(e_load_state::loading);
			if constexpr (_t == e_asset_type::scene)
			{
				if (auto res = load_scene_data(m_path, args))
				{
					m_resource = res.get();
				}
			}
			else if constexpr (_t == e_asset_type::image)
			{
				if (auto res = load_image_data(m_path, args))
				{
					m_resource = res.get();
				}
			}
			else if constexpr (_t == e_asset_type::cubemap)
			{
				if (auto res = load_cubemap_data(m_path, args))
				{
					m_resource = res.get();
				}
			}
			else if constexpr (_t == e_asset_type::shader)
			{
				if (auto res = load_shader_data(m_path, args))
				{
					m_resource = res.get();
				}
			}
			m_time_loadend = time::get_now();
			m_last_args = args;

			// post-load
			// on success, create a mirror flx file
			if (is_load_success)
			{
				set_loadstate(e_load_state::loaded);
				create_flx_file();
				return {};
			}
			else
			{
				set_loadstate(e_load_state::failed_load);
				return result_type::make_error("file at path failed to load as a valid resource.");
			}
			return {};
		}

		inline result<> reload(const load_args& args)
		{
			return load(m_path, args, true);
		}

		inline result<> reload()
		{
			return reload(m_last_args);
		}

		inline bool is_loaded() const
		{
			return m_state == e_load_state::loaded;
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

		inline e_load_state get_loadstate() const
		{
			return m_state;
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
		inline void set_loadstate(e_load_state new_state)
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
	using image_asset = asset_item<e_asset_type::image>;
	using shader_asset = asset_item<e_asset_type::shader>;
	using cubemap_asset = asset_item<e_asset_type::cubemap>;
}