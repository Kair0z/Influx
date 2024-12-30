#include "engine_pch.h"
#include "render_manager.h"

// influx::core
#include "core/log.h"
#include "core/material/material.h"

// influx::engine
#include "content/content_manager.h"
#include "editor/editor_manager.h"

// influx::platform
#include "influx_platform/window.h"

// influx::renderer
#include "influx_renderer.h"
#include "influx_renderer/target.h"

// influx::input
#include "influx_input.h"

// influx::import
#include "influx_import.h"

// imgui
#include "imgui/imgui.h"

namespace influx::engine
{
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
			ImGui::SliderInt("Cullmode: ", (int*)&settings.m_cullmode, 0, 2);

			renderer::set_settings(settings);
		}

		uint32 m_num_pipelines = 0u;
	};
	
	render_manager::render_manager(engine* engine)
		: mp_imgui_drawdata{ nullptr }
		, mp_window_target{ nullptr }
		, mp_scene_target{ nullptr }
	{
		// create renderer
		influx::renderer::init_args render_init_args{};
		render_init_args.m_api_type = influx::renderer::e_render_api::dx12;
		// render_init_args.m_api_type = influx::renderer::e_render_api::vulkan;
		influx::renderer::initialize(render_init_args);

		// window render target:
		cptr<platform::window> window = engine->get_window().get();
		mp_window_target = renderer::acquire_window_target(*window);

		// scene render target:
		influx::renderer::target_create_args target_args{};
		target_args.m_has_depth_stencil = true;
		target_args.m_width = mp_window_target->get_width();
		target_args.m_heigth = mp_window_target->get_height();
		mp_scene_target = influx::renderer::create_target(target_args);
		
		// init imgui:
		initialize_imgui();

		// signal window resize once
		const auto& clientrect = window->get_rect(platform::window::e_space::client);
		on_window_resize(clientrect.get_dimensions());

		// static editor
		editor_manager::static_window<render_editor>("renderer").set_name("renderer");
	}

	render_manager::~render_manager()
	{
		influx::renderer::cleanup();
	}

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
#pragma endregion

	// loads assets from content_manager into the influx::renderer
	void render_manager::load_render_assets(content_manager* cont_man)
	{
		// these are shared 'staging' buffers that are read and copied when 'loaded' to the renderer
		// if we end up wanting to multithread this, we'll have to rework this approach!
		static renderer::shader_data m_shader_data{};
		static renderer::texture_data m_tex_data{};
		static renderer::mesh_data m_mesh_data{};
		static material m_material_data{};

		// CONTENT SECTION
		{
			for (const auto& asset : cont_man->get_scenes())
			{
				if (asset.second.is_loaded() && !asset.second.m_resource.m_meshes.empty())
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

			for (const auto& asset : cont_man->get_images())
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

			for (const auto& asset : cont_man->get_shaders())
			{
				if (renderer::has_shader(asset.first) == false && asset.second.is_loaded())
				{
					const imp::shader_data& vs_shader = asset.second.m_resource;
					translate(vs_shader, m_shader_data);
					influx::renderer::load(asset.first, m_shader_data);
				}
			}
		}
		
		// HARDCODED SECTION
		{
			if (renderer::has_mesh("engine_plane") == false)
			{
				const static math::vectorf3 positions[4u]
				{
					{ 1.0f, 0.0f, 1.0f },
					{ -1.0f, 0.0f, 1.0f },
					{ 1.0f, 0.0f, -1.0f },
					{ -1.0f, 0.0f, -1.0f }
				};
				const static math::vectorf4 colours[4u]
				{
					{ 1.0f, 0.0f, 0.0f, 1.0f },
					{ 0.0f, 1.0f, 0.0f, 1.0f },
					{ 0.0f, 0.0f, 1.0f, 1.0f },
					{ 1.0f, 1.0f, 0.0f, 1.0f }
				};
				const static math::vectorf2 uvs[4u]
				{
					{ 0.0f, 0.0f },
					{ 1.0f, 0.0f },
					{ 1.0f, 1.0f },
					{ 0.0f, 1.0f }
				};
				const static math::vectorf3 normals[4u]
				{
					{ 0.0f, 1.0f, 0.0f },
					{ 0.0f, 1.0f, 0.0f },
					{ 0.0f, 1.0f, 0.0f },
					{ 0.0f, 1.0f, 0.0f }
				};

				m_mesh_data.m_vertices.resize(4u);
				m_mesh_data.m_indices.resize(6u);

				for (uint8 i = 0u; i < 4u; ++i)
				{
					m_mesh_data.m_vertices[i] = {
						.m_position{positions[i]},
						.m_colour{colours[i]},
						.m_normal{normals[i]},
						.m_texcoords{uvs[i]} };
				}

				m_mesh_data.m_indices[0] = 0u;
				m_mesh_data.m_indices[1] = 2u;
				m_mesh_data.m_indices[2] = 1u;
				m_mesh_data.m_indices[3] = 2u;
				m_mesh_data.m_indices[4] = 3u;
				m_mesh_data.m_indices[5] = 1u;

				influx::renderer::load("engine_plane", m_mesh_data);
			}
		}
	}

	void render_manager::record_imgui_frame(const function<void(ImGuiContext&)>& func)
	{
		ImGui::NewFrame();
		func(*ImGui::GetCurrentContext());
		ImGui::Render();
		mp_imgui_drawdata = ImGui::GetDrawData();
	}

	void render_manager::render(const renderer::scene& scene)
	{
		cptr<platform::window> window = get_engine()->get_window().get();
		if (window == nullptr)
		{
			logonce(e_log_category::warning, "render_manager::render: no window to render!");
			return;
		}

		mp_window_target = renderer::acquire_window_target(*window);
		mp_scene_target->resize(*mp_window_target);

		// 1. clear
		renderer::clear_args clear{ {0.2f, 0.2f, 0.2f, 0.2f } };
		renderer::clear_target(*mp_scene_target, clear);
		
		// 2. scene render
		if (scene.has_meshes())
		{
			renderer::draw_scene(scene, *mp_scene_target);
		}

		// 3. debug render
		if (mp_debug_scene != nullptr)
		{
			renderer::draw_debug(*mp_debug_scene, *mp_scene_target);
		}

		// 4. imgui render
		if (mp_imgui_drawdata != nullptr)
		{
			renderer::draw_imgui(mp_imgui_drawdata, *mp_scene_target);
		}
		
		// 5. copy scene-target into window
		influx::renderer::copy_target(*mp_scene_target, *mp_window_target);

		// present to window
		influx::renderer::present_args present_args{};
		present_args.m_vsync = false;
		influx::renderer::present_swapchain(present_args);

		mp_imgui_drawdata = nullptr;
	}

	renderer::scene_debug& render_manager::get_debug_render()
	{
		if (mp_debug_scene == nullptr)
		{
			mp_debug_scene = new renderer::scene_debug();
		}

		return *mp_debug_scene;
	}

	void render_manager::on_window_resize(const math::vectoru2& new_dimensions)
	{
		// update imgui IO
		ImGui::GetIO().DisplaySize = { (float)new_dimensions.x, (float)new_dimensions.y };

		// update renderer
		// ... todo
	}

	void render_manager::initialize_imgui()
	{
		// create ImGui context
		ImGui::CreateContext();

		// Build texture atlas
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

		// docking
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		// mouse events
		input::subscribe([this, &io](const input::mouse_event& ev)
		{
			switch (ev.m_type)
			{
			case input::mouse_event::type::move:
			{
				bool want_absolute_pos = (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0;
				if (want_absolute_pos)
				{
					ImGui::GetIO().AddMousePosEvent(ev.m_position.m_screen.x, ev.m_position.m_screen.y);
				}
				else
				{
					ImGui::GetIO().AddMousePosEvent(ev.m_position.m_client.x, ev.m_position.m_client.y);
				}
			}
			break;

			case input::mouse_event::type::leave:
			{
				io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
			}
			break;

			case input::mouse_event::type::scroll:
			{
				io.AddMouseWheelEvent(0.0f, ev.m_wheel_delta);
			}
			break;

			case input::mouse_event::type::button_down:
			{
				int button_value = 0;
				switch (ev.m_button)
				{
				case input::e_mouse_button::left: button_value = 0; break;
				case input::e_mouse_button::middle: button_value = 2; break;
				case input::e_mouse_button::right: button_value = 1; break;
				}

				io.AddMouseButtonEvent(button_value, true);
			}
			break;

			case input::mouse_event::type::button_up:
			{
				int button_value = 0;
				switch (ev.m_button)
				{
				case input::e_mouse_button::left: button_value = 0; break;
				case input::e_mouse_button::middle: button_value = 2; break;
				case input::e_mouse_button::right: button_value = 1; break;
				}

				io.AddMouseButtonEvent(button_value, false);
			}
			break;
			}
		});
	}
}