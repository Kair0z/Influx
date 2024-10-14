#pragma once

// influx::renderer
namespace influx::renderer
{
	class target;
	struct scene;
}

namespace influx::engine
{
	class render_manager final
	{
	public:
		render_manager(engine* engine);
		~render_manager();

		// loads application asset data into renderer (textures/meshes/shaders)
		void load_render_assets(content_manager* cont_man);

		// renders a frame
		void render(const influx::renderer::scene& scene);

	private:
		influx::renderer::target* mp_window_target;
		influx::renderer::target* mp_scene_target;
	};
}