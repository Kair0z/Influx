#include "engine_pch.h"
#include "render_manager.h"

// influx::core
#include "core/log.h"

// influx::engine
#include "content/content_manager.h"

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
	render_manager::render_manager(engine* engine)
	{
		// create renderer
		influx::renderer::init_args render_init_args{};
		render_init_args.m_api_type = influx::renderer::e_render_api::dx12;
		// render_init_args.m_api_type = influx::renderer::e_render_api::vulkan;
		influx::renderer::initialize(render_init_args);

		// window render target:
		platform::window const* window = engine->get_window();
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
	}

	render_manager::~render_manager()
	{
		influx::renderer::cleanup();
	}

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

	// loads assets from content_manager into the influx::renderer
	void render_manager::load_render_assets(content_manager* cont_man)
	{
		// these are shared staging buffers that are read and copied when 'loaded' to the renderer
		// if we end up wanting to multithread this, we'll have to rework this approach!
		static influx::renderer::shader_data m_shader_data{};
		static influx::renderer::texture_data m_tex_data{};
		static influx::renderer::mesh_data m_mesh_data{};
		static influx::renderer::material m_material_data{};

		for (const auto& asset : cont_man->get_scenes())
		{
			if (renderer::has_mesh(asset.first) == false)
			{
				if (asset.second.is_finished_loading() && !asset.second.m_resource.m_meshes.empty())
				{
					const imp::scene_data::mesh& mesh = asset.second.m_resource.m_meshes[0u];
					translate(mesh, m_mesh_data);
					influx::renderer::load(asset.first, m_mesh_data);
				}
			}
		}

		for (const auto& asset : cont_man->get_images())
		{
			if (renderer::has_texture(asset.first) == false)
			{
				if (asset.second.is_finished_loading())
				{
					translate(asset.second.m_resource, m_tex_data);
					influx::renderer::load(asset.first, m_tex_data);
				}
			}
		}

		// todo: for now only 2 shaders are necessary
		auto load_shader = [](const string& name, const content_manager::shader_item& item)
		{
			if (renderer::has_shader(name) == false)
			{
				const imp::shader_data& vs_shader = item.m_resource;
				translate(vs_shader, m_shader_data);
				influx::renderer::load(name, m_shader_data);
			}
		};
#if 0
		for (const auto& asset : cont_man->get_shaders())
		{
			load_shader(asset.first, asset.second);
		}
#else
		const auto& shaders = cont_man->get_shaders();
		const string& vs_name = "shaders_vs";
		if (shaders.contains(vs_name))
		{
			load_shader(vs_name, shaders.at(vs_name));
		}
		
		const string& ps_name = "shaders_ps";
		if (shaders.contains(ps_name))
		{
			load_shader(ps_name, shaders.at(ps_name));
		}
#endif

		// hardcoded assets
		if (renderer::has_material("mat_transistor") == false)
		{	
			m_material_data.m_basecolor = colour::k_white;
			m_material_data.m_tex_albedo = "T_Sword_Opaque_BC";
			m_material_data.m_tex_normal = "T_Sword_Opaque_N";
			m_material_data.m_tex_roughness = "T_Sword_Opaque_N";
			m_material_data.m_tex_special = "T_Sword_Opaque_N";
			influx::renderer::load("mat_transistor", m_material_data);
		}

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

	void render_manager::record_imgui_frame(const function<void(ImGuiContext&)>& func)
	{
		ImGui::NewFrame();
		func(*ImGui::GetCurrentContext());
		ImGui::Render();
		mp_imgui_drawdata = ImGui::GetDrawData();
	}

	void render_manager::render(const renderer::scene& scene)
	{
		platform::window const* window = get_engine()->get_window();
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

		// 3. imgui render
		if (mp_imgui_drawdata != nullptr)
		{
			renderer::draw_imgui(mp_imgui_drawdata, *mp_scene_target);
		}
		
		// 4. copy scene-target into window
		influx::renderer::copy_target(*mp_scene_target, *mp_window_target);

		// present to window
		influx::renderer::present_args present_args{};
		present_args.m_vsync = true;
		influx::renderer::present_swapchain(present_args);

		mp_imgui_drawdata = nullptr;
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

		// mouse events
		input::subscribe([this, &io](const input::mouse_event& ev)
		{
			switch (ev.m_type)
			{
			case input::mouse_event::e_type::move:
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

			case input::mouse_event::e_type::leave:
			{
				io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
			}
			break;

			case input::mouse_event::e_type::scroll:
			{
				io.AddMouseWheelEvent(0.0f, ev.m_wheel_delta);
			}
			break;

			case input::mouse_event::e_type::button_down:
			{
				int button_value = 0;
				switch (ev.m_button)
				{
				case input::mouse_event::e_button::left: button_value = 0; break;
				case input::mouse_event::e_button::middle: button_value = 2; break;
				case input::mouse_event::e_button::right: button_value = 1; break;
				}

				io.AddMouseButtonEvent(button_value, true);
			}
			break;

			case input::mouse_event::e_type::button_up:
			{
				int button_value = 0;
				switch (ev.m_button)
				{
				case input::mouse_event::e_button::left: button_value = 0; break;
				case input::mouse_event::e_button::middle: button_value = 2; break;
				case input::mouse_event::e_button::right: button_value = 1; break;
				}

				io.AddMouseButtonEvent(button_value, false);
			}
			break;
			}
		});
	}
}