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

	class rendergraph_editor final : public editor::editor_window
	{
	public:
		virtual void on_run() override
		{
			// rendergraph info
			const auto& rendergraph_info = renderer::get_rendergraph_info();
			ImGui::Text("resources:");
			for (const auto& texture : rendergraph_info.m_textures)
			{
				ImGui::Text("texture: %s", texture.m_name.c_str());
			}
			for (const auto& buffer : rendergraph_info.m_buffers)
			{
				ImGui::Text("buffer: %s", buffer.m_name.c_str());
			}
		}
	};

	class render_editor final : public editor::editor_window
	{
	public:
		virtual void on_run() override
		{
			// pipeline info
			const auto& pipeline_info = renderer::get_pipeline_info();
			ImGui::Text("num_pipelines: %i", pipeline_info.m_num_pipelines);

			// memory info
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
	
	render_manager::render_manager()
		: m_imgui{}
	{
		// create renderer
		influx::renderer::init_args render_init_args{};
		render_init_args.m_api_type = influx::renderer::e_render_api::dx12;
		// render_init_args.m_api_type = influx::renderer::e_render_api::vulkan;
		render_init_args.m_log_func = [](renderer::e_log, const char* message)
		{
			engine::log(e_log_category::info, message);
		};
		const string& engine_assets_directory = to_string(get_engine_directory(engine_directory::assets).get_full_path());
		render_init_args.m_shader_source_folder = engine_assets_directory + "/engine/shaders/";
		influx::renderer::initialize(render_init_args);

		// static editor
		editor::editor_manager::static_window<render_editor>("renderer").set_name("renderer");
		editor::editor_manager::static_window<rendergraph_editor>("rendergraph").set_name("rendergraph");
	}

	render_manager::~render_manager()
	{
		m_views.clear();

		influx::renderer::cleanup();
	}

	void render_manager::on_window_resize(const math::vectoru2& new_dimensions)
	{
		m_imgui.on_window_resize(new_dimensions);
	}

	void render_manager::render()
	{
		renderer::start_frame();
		
		// render each view
		for (auto& pair : m_views)
		{
			render_view_id id = pair.first;
			render_view& view = pair.second;

			if (view.should_render() == false)
				continue;

			// cannot render invalid dimensions
			if (view.has_valid_dimensions() == false)
				continue;

			// create target if first time
			if (view.m_target == nullptr)
			{
				renderer::target_create_args args{};
				args.m_has_colour = true;
				args.m_has_depth_stencil = true;
				args.m_width = view.m_dimensions.x;
				args.m_heigth = view.m_dimensions.y;
				view.m_target = renderer::create_target(args);
			}

			view.m_target->resize(view.m_dimensions);

			// render view
			if (view.is_valid())
			{
				// update camera
				view.get_scene().set_camera(
					view.get_camera(), view.get_camera_transform().get_matrix());
				
				renderer::clear_target(view.get_target(), { .m_colour = view.m_clear_colour });

				renderer::scene& view_scene = view.get_scene();
				view_scene.set_debug_render_enabled(true);
				renderer::draw_scene(view_scene, view.get_target());

				++view.m_frame_counter;
			}
		}

		// render imgui
		// (internally renders into potentially multiple windows backbuffers)
		renderer::scene_imgui& imgui_scene = get_imgui_scene();
		if (!imgui_scene.is_empty() && is_imgui_render_enabled())
		{
			m_imgui.render(imgui_scene);
		}

		// submits all frame gpu commands to the GPU
		influx::renderer::end_frame();

		// presents each registered swapchain
		renderer::present_all({ .m_vsync = false });
	}

	void render_manager::stream_content(const content_manager& cont_man)
	{
		m_streamer.stream(cont_man);
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

	render_view& render_manager::get_renderview(e_render_view view)
	{
		return m_views[k_render_view_names[static_cast<uint8>(view)]];
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
}