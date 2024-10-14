#pragma once

// influx::core
#include "core/container/map.h"

// influx::import
#include "influx_import.h"

namespace influx::engine
{
	class content_manager final
	{
	public:
		content_manager(engine* engine);
		~content_manager();

		const map<string, imp::scene_data>& get_scenes() const;
		const map<string, imp::image_data>& get_images() const;
		const map<string, imp::shader_data>& get_shaders() const;

	private:
		map<string, imp::scene_data> m_scenes;
		map<string, imp::image_data> m_images;
		map<string, imp::shader_data> m_shaders;
	};
}