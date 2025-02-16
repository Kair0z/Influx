#include "engine_pch.h"
#include "render_manager.h"

// influx::core
#include "core/log.h"
#include "core/material/material.h"

// influx::engine
#include "content/content_manager.h"
#include "editor/editor_manager.h"
#include "file/engine_files.h"
#include "window/window_manager.h"

// influx::platform
#include "influx_platform/window.h"

// influx::renderer
#include "influx_renderer.h"
#include "influx_renderer/target.h"

// influx::import
#include "influx_import.h"

namespace influx::engine
{
#pragma region translation
	void translate(const imp::scene_data::mesh& imp_data, renderer::mesh_data& out_data);
	void translate(const imp::shader_data& imp_data, renderer::shader_data& out_data);
	void translate(const imp::image_data& imp_data, renderer::texture_data& out_data);
	void translate(const shader::compile_output& shader_data, renderer::shader_data& out_data);
#pragma endregion

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
			ImGui::Checkbox("wireframe: ", &settings.m_wireframe);
			ImGui::Checkbox("debug render: ", &g_global_settings.m_render_debug);
			ImGui::SliderInt("cullmode: ", (int*)&settings.m_cullmode, 0, 2);
			ImGui::ColorEdit3("clear colour: ", &g_global_settings.m_clearcolour.r);

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
		platform::window& main_window = get_engine()->get_window();
		renderer::target* window_target = renderer::get_window_target(main_window);

		// starts the underlying rendergraph
		renderer::start_frame();
		{
			mp_scene_target->resize(*window_target);

			renderer::clear_target(*mp_scene_target, {});

			// scene render
			if (scene.is_empty() == false)
			{
				renderer::draw_scene(scene, *mp_scene_target);
			}

			// shadertoy render
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

			// debug render
			if (debug.is_empty() == false && get_render_debug())
			{
				renderer::draw_debug(debug, *mp_scene_target);
			}

			// imgui render
			if (imgui.is_empty() == false)
			{
				ImGui::NewFrame();
				m_imgui.new_frame();
				imgui.m_imgui_stacks[0u](*ImGui::GetCurrentContext());
				ImGui::Render();
				m_imgui.render();
			}

			// scene target to window target
			influx::renderer::copy_target(*mp_scene_target, *window_target);
		}

		// submits the gpu commands
		influx::renderer::end_frame();

		// present the window backbuffers
		renderer::present(main_window, {.m_vsync = false });
	}

	bool render_manager::has_shader_loaded(const shader::shader_signature& signature) const
	{
		return influx::renderer::has_shader(signature);
	}

	bool render_manager::has_mesh_loaded(const string& name) const
	{
		return influx::renderer::has_mesh(name);
	}

	bool render_manager::has_texture_loaded(const string& name) const
	{
		return influx::renderer::has_texture(name);
	}

	void* render_manager::get_loaded_texture_id(const string& name) const
	{
		if (has_texture_loaded(name))
		{
			return influx::renderer::get_imgui_texture_id(name);
		}
		else
		{
			return 0u;
		}
	}

	bool render_manager::get_render_debug() const
	{
		return g_global_settings.m_render_debug;
	}

#pragma region content_streaming
#pragma region translation layer
	void translate(const imp::scene_data::mesh& imp_data, renderer::mesh_data& out_data)
	{
		out_data.m_indices.resize(imp_data.m_indices.size());
		out_data.m_vertices.resize(imp_data.m_positions.size());

		for (uint64 i = 0u; i < imp_data.m_positions.size(); ++i)
		{
			out_data.m_vertices[i].m_position = imp_data.m_positions[i];
			out_data.m_vertices[i].m_normal = imp_data.m_normals[i];
			out_data.m_vertices[i].m_texcoords = imp_data.m_uvs[i];
			// ...
		}

		for (uint64 i = 0u; i < imp_data.m_indices.size(); ++i)
		{
			out_data.m_indices[i] = imp_data.m_indices[i];
		}
	}

	renderer::shader_data translate(const imp::shader_data& imp_data)
	{
		renderer::shader_data result{};
		result.m_bytecode.resize(imp_data.m_compile_result.m_bytecode.size());

		for (uint64 i = 0u; i < imp_data.m_compile_result.m_bytecode.size(); ++i)
		{
			result.m_bytecode[i] = imp_data.m_compile_result.m_bytecode[i];
		}

		result.m_type = imp_data.m_type;
		result.m_reflection = imp_data.m_compile_result.m_reflection;
		return result;
	}

	void translate(const imp::shader_data& imp_data, renderer::shader_data& out_data)
	{
		out_data.m_bytecode.resize(imp_data.m_compile_result.m_bytecode.size());

		for (uint64 i = 0u; i < imp_data.m_compile_result.m_bytecode.size(); ++i)
		{
			out_data.m_bytecode[i] = imp_data.m_compile_result.m_bytecode[i];
		}

		out_data.m_type = imp_data.m_type;
		out_data.m_reflection = imp_data.m_compile_result.m_reflection;
	}

	void translate(const imp::image_data& imp_data, renderer::texture_data& out_data)
	{
		out_data.m_pixels.resize(imp_data.m_pixels.size());

		for (uint64 i = 0u; i < imp_data.m_pixels.size(); ++i)
		{
			out_data.m_pixels[i] = imp_data.m_pixels[i];
		}

		out_data.m_width = imp_data.m_dimensions.x;
	}

	void translate(const shader::compile_output& shader_data, renderer::shader_data& out_data)
	{
		out_data.m_bytecode.resize(shader_data.m_bytecode.size());

		for (uint64 i = 0u; i < shader_data.m_bytecode.size(); ++i)
		{
			out_data.m_bytecode[i] = shader_data.m_bytecode[i];
		}

		out_data.m_type = shader_data.m_signature.m_type;
		out_data.m_reflection = shader_data.m_reflection;
	}
#pragma endregion

	// staging buffers
	static renderer::shader_data m_shader_data{};
	static renderer::texture_data m_tex_data{};
	static renderer::mesh_data m_mesh_data{};
	static material m_material_data{};

	// loads assets from content_manager into the influx::renderer
	void render_manager::stream_content(const content_manager& cont_man)
	{
		stream_shaders(cont_man);
		stream_images(cont_man);
		stream_meshes(cont_man);
	}

	void render_manager::stream_shaders(const content_manager& content)
	{
		for (const auto& asset : content.get_shaders())
		{
			if (asset.second.is_loaded())
			{
				const imp::shader_data shader_data = asset.second.m_resource;
				const shader::shader_signature& signature = shader_data.m_signature;

				const time::point render_load_time = renderer::get_time_loaded_shader(signature);
				if (!renderer::has_shader(signature) || asset.second.m_time_loadend > render_load_time)
				{
					// if load time is newer than previous render load time, reload
					influx::renderer::load(signature, translate(shader_data), true);
				}
			}
		}
	}

	void render_manager::stream_images(const content_manager& content)
	{
		for (const auto& asset : content.get_images())
		{
			if (renderer::has_texture(asset.first) == false)
			{
				if (asset.second.is_loaded())
				{
					translate(asset.second.m_resource, m_tex_data);
					influx::renderer::load(asset.first, m_tex_data);
				}
			}
		}
	}

	void render_manager::stream_meshes(const content_manager& content)
	{
		// content meshes
		for (const auto& asset : content.get_scenes())
		{
			if (asset.second.is_loaded())
			{
				for (uint32 i = 0u; i < asset.second.m_resource.get_num_meshes(); ++i)
				{
					const imp::scene_data::mesh& mesh = asset.second.m_resource.get_mesh(i);
					const string name = asset.first + "_" + std::to_string(i);

					if (renderer::has_mesh(name) == false)
					{
						translate(mesh, m_mesh_data);
						influx::renderer::load(name, m_mesh_data);
					}
				}
			}
		}

		// inline meshes
		if (renderer::has_mesh("engine_plane") == false)
		{
			influx::renderer::load("eplane", renderer::get_inline_mesh_plane());
		}
		if (renderer::has_mesh("engine_box") == false)
		{
			influx::renderer::load("ebox", renderer::get_inline_mesh_box());
		}
		if (renderer::has_mesh("engine_sphere") == false)
		{
			influx::renderer::load("esphere", renderer::get_inline_mesh_sphere());
		}
	}
#pragma endregion
}