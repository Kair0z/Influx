#pragma once

// influx::core
#include "core/macros.h"
#include "core/math/vector.h"

// influx::renderer
namespace influx::renderer
{
	class target;
	struct scene;
	struct scene_debug;
}

// ImGui
struct ImDrawData;
struct ImGuiContext;

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
		void stream_content(content_manager* cont_man);

		void record_imgui_frame(const function<void(ImGuiContext&)>& func);

		void render(const renderer::scene&);

		void on_window_resize(const math::vectoru2& new_dimensions);

		renderer::scene_debug& get_debug_render();

		// textures
		bool has_texture_loaded(const string& name) const;
		void* get_loaded_texture_id(const string& name) const;

	private:
		renderer::target* mp_window_target;
		renderer::target* mp_scene_target;
		ImDrawData* mp_imgui_drawdata = nullptr;
		renderer::scene_debug* mp_debug_scene = nullptr;

		void stream_shaders(const content_manager& content);
		void stream_images(const content_manager& content);
		void stream_meshes(const content_manager& content);

		void initialize_imgui();
	};
}