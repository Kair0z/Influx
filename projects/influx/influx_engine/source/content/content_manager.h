#pragma once

// influx::core
#include "core/container/map.h"
#include "core/time.h"

// influx::import
#include "influx_import.h"

namespace influx::engine
{
	class content_manager final
	{
		enum class e_load_state : uint8
		{
			unloaded,
			loading,
			loaded,
			count
		};

		template <typename _t>
		struct content_item final
		{
			_t m_item;
			e_load_state m_state;
		};

		using scene_item = content_item<imp::scene_data>;
		using image_item = content_item<imp::image_data>;
		using shader_item = content_item<imp::shader_data>;

	public:
		content_manager(engine* engine);
		~content_manager();

		const map<string, scene_item>& get_scenes() const;
		const map<string, image_item>& get_images() const;
		const map<string, shader_item>& get_shaders() const;

		// loads the /influx/resources/ folder
		void load_engine_resources(engine* engine);

	private:
		map<string, scene_item> m_scenes;
		map<string, image_item> m_images;
		map<string, shader_item> m_shaders;

		thread m_loading_thread;

		time::point m_start_engine_resources;

		bool m_is_loading = false;
	};
}