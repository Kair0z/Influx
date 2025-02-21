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

	class render_editor final : public editor_window
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

	class shadertoy_editor final : public editor_window
	{
	public:
		const char* filepath = "D:/Git/Influx/assets/Shaders/shadertoy.hlsl";

		shadertoy_editor()
		{
			m_texteditor.SetHandleKeyboardInputs(true);
			m_texteditor.SetHandleMouseInputs(true);
			m_texteditor.SetReadOnly(false);

			// const string& file_content = textfile::read_all(string(filepath));
			// m_texteditor.InsertText(file_content.c_str());
		}

		shader::compile_args make_compile_args(shader::e_shader_type type, const string& entrypoint)
		{
			shader::compile_args compile_args{};
			compile_args.m_signature.m_target = shader::e_shader_target::_6_6;
#if INFLUX_DEBUG
			compile_args.m_compile_debug = true;
#else
			compile_args.m_compile_debug = false;
#endif
			compile_args.m_pbd = true;
			compile_args.m_reflection = true;
			compile_args.m_defines = {};
			compile_args.m_pdb_folder = get_engine_directory(engine_directory::intermediate).m_path_full + "/shaderdebug/";
			compile_args.m_include_folder = get_engine_directory(engine_directory::assets).m_path_full + "/shaders/include/";
			compile_args.m_signature.m_type = type;
			compile_args.m_signature.m_entrypoint = entrypoint;
			return compile_args;
		}

		virtual void on_run() override
		{
			// render text editor
			m_texteditor.SetLanguageDefinition(imgui::TextEditor::LanguageDefinition::HLSL());
			m_texteditor.Render("file");

			// render log
			string log_text = {};
			for (const string& str : m_compile_errors)
			{
				log_text.append(str + "\n");
			}
			ImVec4 TextColor = m_compile_errors.size() > 0u ? ImVec4(1, 0, 0, 1) : ImVec4(1, 1, 1, 1);
			ImGui::TextColored(TextColor, log_text.c_str());

			// buttons
			if (ImGui::Button("compile"))
			{
				const string source = m_texteditor.GetText();
				m_compiled_vs = shader::compile_shader_source(source, make_compile_args(shader::e_shader_type::vs, "main_vs"));
				m_compiled_ps = shader::compile_shader_source(source, make_compile_args(shader::e_shader_type::ps, "main_ps"));
				m_compile_errors = merged(m_compiled_vs.m_log, m_compiled_ps.m_log);
				m_compile_success = m_compiled_vs.m_success && m_compiled_ps.m_success;
			}

			if (ImGui::Button("render"))
			{
				m_enable_render = m_compile_success && !m_enable_render;
			}
		}

		bool can_render() const
		{
			return m_enable_render && m_compile_success;
		}

		const shader::compile_output& get_compiled_vs() const { return m_compiled_vs; }
		const shader::compile_output& get_compiled_ps() const { return m_compiled_ps; }

	private:
		imgui::TextEditor m_texteditor{};
		shader::compile_output m_compiled_vs{};
		shader::compile_output m_compiled_ps{};
		bool m_compile_success = false;
		bool m_enable_render = false;
		vector<string> m_compile_errors{};
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
		editor_manager::static_window<render_editor>("renderer").set_name("renderer");
		editor_manager::static_window<shadertoy_editor>("shadertoy").set_name("shadertoy");
	}

	render_manager::~render_manager()
	{
		influx::renderer::cleanup();
	}

	void render_manager::on_window_resize(const math::vectoru2& new_dimensions)
	{
		m_imgui.on_window_resize(new_dimensions);
	}

	void render_manager::render(
		const renderer::scene& scene,
		const renderer::scene2D& scene2D,
		const renderer::scene_imgui& imgui, 
		const renderer::scene_debug& debug)
	{
		const platform::window& main_window = get_engine()->get_window();
		renderer::target* window_target = renderer::get_window_target(main_window);

		// starts the underlying rendergraph
		renderer::start_frame();
		{
			mp_scene_target->resize(*window_target);

			renderer::clear_target(*mp_scene_target, {.m_colour = g_global_settings.m_clearcolour });

			if (scene.is_empty() == false)
			{
				renderer::draw_scene(scene, *mp_scene_target);
			}
			if (debug.is_empty() == false && is_debug_render_enabled())
			{
				renderer::draw_debug(debug, *mp_scene_target);
			}
#if 0
			shadertoy_editor& editor = editor_manager::static_window<shadertoy_editor>("shadertoy");
			if (editor.can_render())
			{
				// flags the renderer to overwrite previous shader on this name
				const bool reload = true;

				renderer::shader_data vs_data{};
				shader::shader_signature vs_signature{};
				vs_signature.m_entrypoint = "shadertoy_vs";
				translate(editor.get_compiled_vs(), vs_data);
				renderer::load(vs_signature, vs_data, reload);

				renderer::shader_data ps_data{};
				shader::shader_signature ps_signature{};
				ps_signature.m_entrypoint = "shadertoy_ps";
				translate(editor.get_compiled_ps(), ps_data);
				renderer::load(ps_signature, ps_data, reload);

				renderer::scene_shadertoy shadertoy{};
				renderer::draw_shadertoy(shadertoy, *mp_scene_target);
			}
#endif

			// scene target -> main window target
			influx::renderer::copy_target(*mp_scene_target, *window_target);

			// imgui render (renders straight to its own created window targets)
			if (imgui.is_empty() == false && is_imgui_render_enabled())
			{
				m_imgui.render(imgui);
			}
		}

		// submits all gpu commands
		influx::renderer::end_frame();

		// present each swapchain registered
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
}