#pragma once

// influx::core
#include "core/container/map.h"
#include "core/time.h"

// influx::import
#include "influx_import.h"

namespace influx::engine
{
	class engine;
}

namespace influx::engine
{
	// content in influx::engine
	// differentiates between 2 terms.
	// - resources: the raw input items that the engine accepts and can convert into assets
	// - assets: influx-native representations of resources (.flx)
	class content_manager final
	{
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

		template <typename _t>
		struct asset_item final
		{
			_t m_resource; // the raw resource
			e_load_state m_state;
			e_asset_origin m_origin;
		};

		using scene_item = asset_item<imp::scene_data>;
		using image_item = asset_item<imp::image_data>;
		using shader_item = asset_item<imp::shader_data>;

	public:
		content_manager(engine* engine);
		~content_manager();

		const map<string, scene_item>& get_scenes() const;
		const map<string, image_item>& get_images() const;
		const map<string, shader_item>& get_shaders() const;

		// loads /influx/assets/
		void load_engine_assets(engine* engine);

		// loads /influx/games/'game_name'/assets/
		void load_game_assets(const string& game_name, engine* engine);

	private:
		map<string, scene_item> m_scenes;
		map<string, image_item> m_images;
		map<string, shader_item> m_shaders;

		thread m_loading_thread;

		time::point m_start_engine_resources;

		bool m_is_loading = false;

		void load_assets(engine* engine, e_asset_origin, const file& root);
	};
}