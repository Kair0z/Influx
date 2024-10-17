#pragma once

#include "core/math/vector.h"

// influx::renderer
namespace influx::renderer
{
	class target;
	struct scene;
}

struct ImDrawData;

namespace influx::engine
{
	class render_manager final
	{
	public:
		render_manager(engine* engine);
		~render_manager();

		// loads application asset data into renderer (textures/meshes/shaders)
		void load_render_assets(content_manager* cont_man);

		void record_imgui_frame(const function<void()>& func);

		void render(influx::renderer::scene const* scene);

		void on_window_resize(const math::vectoru2& new_dimensions);

	private:
		influx::renderer::target* mp_window_target;
		influx::renderer::target* mp_scene_target;

		ImDrawData* mp_imgui_drawdata = nullptr;

		void initialize_imgui();
	};
}