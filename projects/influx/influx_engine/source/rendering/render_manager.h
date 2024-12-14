#pragma once

#include "core/macros.h"
#include "core/math/vector.h"

#pragma region declarations
// influx::renderer
namespace influx::renderer
{
	class target;
	struct scene;
}

struct ImDrawData;
struct ImGuiContext;
#pragma endregion

namespace influx::engine
{
	class engine;
	class content_manager;

	class render_manager final
	{
		INFLUX_NO_MOVE(render_manager);
		INFLUX_NO_COPY(render_manager);

	public:
		render_manager(engine* engine);
		~render_manager();

		// loads assets from content_manager into the influx::renderer
		void load_render_assets(content_manager* cont_man);

		void record_imgui_frame(const function<void(ImGuiContext&)>& func);

		void render(const renderer::scene&);

		void on_window_resize(const math::vectoru2& new_dimensions);

	private:
		renderer::target* mp_window_target;
		renderer::target* mp_scene_target;
		ImDrawData* mp_imgui_drawdata;

		void initialize_imgui();
	};
}