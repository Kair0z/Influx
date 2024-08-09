#pragma once

#include "core/platform/window.h"

namespace influx::renderer
{
	class target;
	struct scene;
}

namespace influx::application
{
	class content_manager;

	class renderer final
	{
	public:
		renderer(platform::window_handle window_handle);

		// loads application asset data into renderer (textures/meshes/shaders)
		void load_render_assets(content_manager* cont_man);

		void render(const influx::renderer::scene& scene);

	private:
		platform::window_handle m_window_handle;
		influx::renderer::target* mp_window_target;
		influx::renderer::target* mp_scene_color_target;
	};
}