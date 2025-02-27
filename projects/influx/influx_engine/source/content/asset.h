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
	struct shader_data;
	struct scene_data;
	struct image_data;
	struct cubemap_data;
	struct shader_data;
}

// influx::shader
namespace influx::shader
{
	struct compile_args;
}

namespace influx::engine
{
	imp::scene_data load_scene_data(const string& path, const imp::scene_load_args& args);
	imp::image_data load_image_data(const string& path, const imp::image_load_args& args);
	imp::cubemap_data load_cubemap_data(const string& path, const imp::cubemap_load_args& args);
	imp::shader_data load_shader_data(const string& path, const shader::compile_args& args);

	enum class e_asset_type : uint8
	{
		scene,
		image,
		cubemap,
		shader,
		count
	};

	template <e_asset_type _t>
	using data_type = std::tuple_element_t<static_cast<uint64>(_t), std::tuple<
		imp::scene_data,
		imp::image_data,
		imp::cubemap_data,
		imp::shader_data>>;

	template <e_asset_type _t>
	using load_args = std::tuple_element_t<static_cast<uint64>(_t), std::tuple<
		imp::scene_load_args,
		imp::image_load_args,
		imp::cubemap_load_args,
		shader::compile_args>>;

	enum class e_asset_origin : uint8
	{
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
		count
	};

	template <e_asset_type _t>
	struct asset_item final
	{
		using load_args = load_args<_t>;
		using data_type = data_type<_t>;

		data_type m_resource{}; // the raw resource
		e_load_state m_state{};
		e_asset_origin m_origin{};
		string m_name;
		string m_path;
		load_args m_last_args{};

		time::point m_time_loadstart{};
		time::point m_time_loadend{};

		inline asset_item()
		{
			set_loadstate(e_load_state::unloaded);
		}

		inline void load(const string& path, const load_args& args, bool reload = false)
		{
			// use reload instead
			if (get_loadstate() == e_load_state::count) return;
			if (get_loadstate() == e_load_state::loading) return;
			if (get_loadstate() == e_load_state::loaded && reload == false) return;

			engine::log(e_log_category::info, "content:loading {}", get_friendly_name(path).c_str());

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
			else if constexpr (_t == e_asset_type::cubemap)
			{
				m_resource = load_cubemap_data(m_path, args);
			}
			else if constexpr (_t == e_asset_type::shader)
			{
				m_resource = load_shader_data(m_path, args);
			}
			set_loadstate(e_load_state::loaded);
			m_time_loadend = time::get_now();
			m_last_args = args;
		}

		inline void reload(const load_args& args)
		{
			load(m_path, args, true);
		}

		inline void reload()
		{
			reload(m_last_args);
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

		inline float get_load_ms() const
		{
			return time::get_ms_between<float>(m_time_loadend, m_time_loadstart);
		}
	};

	using scene_asset = asset_item<e_asset_type::scene>;
	using image_asset = asset_item<e_asset_type::image>;
	using shader_asset = asset_item<e_asset_type::shader>;
	using cubemap_asset = asset_item<e_asset_type::cubemap>;
}