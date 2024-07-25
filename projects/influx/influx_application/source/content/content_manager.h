#pragma once

// core
#include "core/string.h"
#include "core/container/map.h"

// assets
#include "influx_assets.h"

namespace influx::application
{
	class content_manager final
	{
	public:
		content_manager(const string& resource_dir);

		const map<string, assets::scene_data>& get_scenes() const;
		const map<string, assets::image_data>& get_images() const;
		const map<string, assets::shader_data>& get_shaders() const;

	private:
		map<string, assets::scene_data> m_scenes;
		map<string, assets::image_data> m_images;
		map<string, assets::shader_data> m_shaders;
	};
}