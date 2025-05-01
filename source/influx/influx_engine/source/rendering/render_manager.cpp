#include "engine_pch.h"
#include "render_manager.h"

// influx::core
#include "core/log.h"

// influx::engine
#include "editor/editor_manager.h"
#include "file/engine_files.h"
#include "window/window_manager.h"

// influx::platform
#include "influx_platform/window.h"

// influx::renderer
#include "influx_renderer.h"
#include "influx_renderer/target.h"

namespace influx::engine
{
#pragma region editors
	struct
	{
		bool m_render_debug;
		math::colour_rgba m_clearcolour{};

	} g_global_settings{};

	class render_editor final : public editor::editor_window
	{
	public:
		virtual void on_run() override
		{
			const auto& pipeline_info = renderer::get_pipeline_info();
			ImGui::Text("num_pipelines: %i", pipeline_info.m_num_pipelines);

			const auto& memory_info = renderer::get_memory_info();
			const float video_mem_used = memory_info.m_gpu_usage / (float)(1024 * 1024 * 1024);
			const float video_mem_budget = memory_info.m_gpu_budget / (float)(1024 * 1024 * 1024);
			ImGui::Text("memory: %.2f/%.2fMB", video_mem_used, video_mem_budget);

			renderer::render_settings settings = renderer::get_settings();
			
			if (renderer::can_draw_scene())
			{
				ImGui::Checkbox("wireframe: ", &settings.m_wireframe);
				ImGui::SliderInt("cullmode: ", (int*)&settings.m_cullmode, 0, 2);
				ImGui::ColorEdit3("clear colour: ", &g_global_settings.m_clearcolour.r);
			}

			if (renderer::can_draw_debug())
			{
				ImGui::Checkbox("debug render: ", &g_global_settings.m_render_debug);
			}
			
			renderer::set_settings(settings);
		}

		uint32 m_num_pipelines = 0u;
	};
#pragma endregion
	
	render_manager::render_manager(engine* engine)
		: m_imgui{}
		, mp_window_target{ nullptr }
		, mp_scene_target{ nullptr }
	{
		// create renderer
		influx::renderer::init_args render_init_args{};
		render_init_args.m_api_type = influx::renderer::e_render_api::dx12;
		// render_init_args.m_api_type = influx::renderer::e_render_api::vulkan;
		render_init_args.m_shader_source_folder = get_engine_directory(engine_directory::assets).m_path_full + "/engine/shaders/";
		influx::renderer::initialize(render_init_args);

		// window render target:
		platform::window& window = engine->get_window();
		mp_window_target = renderer::get_window_target(window);

		// scene render target:
		influx::renderer::target_create_args target_args{};
		target_args.m_has_depth_stencil = true;
		target_args.m_width = mp_window_target->get_width();
		target_args.m_heigth = mp_window_target->get_height();
		mp_scene_target = influx::renderer::create_target(target_args);
		mp_scene_target->set_name("scene_target");

		// signal window resize once
		const auto& clientrect = window.get_rect(platform::window::e_space::client);
		on_window_resize(clientrect.get_dimensions());

		// static editor
		editor::editor_manager::static_window<render_editor>("renderer").set_name("renderer");
	}

	render_manager::~render_manager()
	{
		influx::renderer::cleanup();
	}

	void render_manager::on_window_resize(const math::vectoru2& new_dimensions)
	{
		m_imgui.on_window_resize(new_dimensions);
	}

	void render_manager::render()
	{
		renderer::start_frame();
		
		// render the various views
		for (auto& pair : m_views)
		{
			render_view_id id = pair.first;
			render_view& view = pair.second;
			if (view.is_valid() == false) continue;

			view.m_scene.set_debug_render_enabled(true);
			renderer::clear_target(view.get_target(), { .m_colour{1,0,0,1} });
			renderer::draw_scene(view.get_scene(), view.get_target());
			// renderer::draw_2D(view.get_scene2D(), view.get_target());

			view.m_target;
			++view.m_frame_counter;
		}

		// imgui render (renders straight to window backbuffer)
		renderer::scene_imgui& imgui_scene = get_scene_imgui();
		if (!imgui_scene.is_empty() && is_imgui_render_enabled())
		{
			m_imgui.render(imgui_scene);
		}

		// submits all gpu commands to the GPU
		influx::renderer::end_frame();

		// presents each registered swapchain
		renderer::present_all({ .m_vsync = false });
	}

	void render_manager::stream_content(const content_manager& cont_man)
	{
		m_streamer.stream(cont_man);
	}

	void* render_manager::get_loaded_texture_id(const string& name) const
	{
		return m_streamer.get_loaded_texture_id(name);
	}

	bool render_manager::is_debug_render_enabled() const
	{
		const bool user = g_global_settings.m_render_debug;
		const bool render = renderer::can_draw_debug();
		return render && user;
	}
	bool render_manager::is_imgui_render_enabled() const
	{
		const bool render = renderer::can_draw_imgui();
		return render;
	}
	bool render_manager::is_scene_render_enabled() const
	{
		const bool render = renderer::can_draw_scene();
		return render;
	}
	renderer::scene& render_manager::get_scene()
	{
		return m_scene;
	}
	renderer::scene2D& render_manager::get_scene2D()
	{
		return m_scene2D;
	}
	renderer::scene_imgui& render_manager::get_scene_imgui()
	{
		return m_imgui_scene;
	}

	render_view& render_manager::get_renderview(e_render_view view)
	{
		
	}

	render_view& render_manager::get_renderview(const render_view_id& id, const math::vectoru2& size)
	{
		influx_assert(size.x > 0 && size.y > 0);

		renderer::target_create_args create_args{};
		create_args.m_has_colour = true;
		create_args.m_has_depth_stencil = false;
		create_args.m_width = size.x;
		create_args.m_heigth = size.y;

		// create for the first time, or recreate if dimensions are different
		if (!m_views.contains(id) || !m_views[id].is_valid())
		{
			m_views[id].m_target = renderer::create_target(create_args);
		}
		else
		{
			uint32 width = m_views[id].m_target->get_width();
			uint32 height = m_views[id].m_target->get_height();
			if (width != size.x || height != size.y)
			{
				// recreate target
				delete m_views[id].m_target;
				m_views[id].m_target = renderer::create_target(create_args);
			}
		}

		return m_views[id];
	}

	render_view::render_view(const renderer::target_create_args& create_args)
	{
		m_target = renderer::create_target(create_args);
	}
	render_view::~render_view()
	{
		delete m_target;
	}
	const renderer::target& render_view::get_target() const
	{
		return *m_target;
	}
	renderer::scene& render_view::get_scene()
	{
		return m_scene;
	}
	renderer::scene2D& render_view::get_scene2D()
	{
		return m_scene2D;
	}
}