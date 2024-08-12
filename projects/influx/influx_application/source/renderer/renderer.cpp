#include "app_pch.h"
#include "renderer.h"

#include "content/content_manager.h"

// influx::renderer
#include "influx_renderer.h"
#include "influx_renderer/target.h"

// imgui
#include "imgui/imgui.h"

// influx::input
#include "influx_input.h"

namespace influx::application
{
	renderer::renderer(platform::window_handle window_handle)
		: m_window_handle{ window_handle }
	{
		// create renderer
		influx::renderer::init_args render_init_args{};
		render_init_args.m_api_type = influx::renderer::e_render_api::dx12;
		influx::renderer::initialize(render_init_args);

		// window render target:
		mp_window_target = influx::renderer::get_window_target(m_window_handle);

		// scene render target:
		influx::renderer::target_create_args target_args{};
		target_args.m_has_depth_stencil = true;
		target_args.m_width = mp_window_target->get_width();
		target_args.m_heigth = mp_window_target->get_height();
		mp_scene_color_target = influx::renderer::create_target(target_args);

		// create ImGui context
		ImGui::CreateContext();

		// Build texture atlas
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

		input::subscribe([this, &io](const input::mouse_event& ev)
		{
			switch (ev.m_type)
			{
			case input::mouse_event::e_type::move:
			{
				bool want_absolute_pos = (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0;
				if (want_absolute_pos)
				{
					ImGui::GetIO().AddMousePosEvent(ev.m_position_screen.x, ev.m_position_screen.y);
				}
				else
				{
					ImGui::GetIO().AddMousePosEvent(ev.m_position_client.x, ev.m_position_client.y);
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

	inline ImDrawData* get_imgui_data(const influx::renderer::target& target)
	{
		ImGui::GetIO().DisplaySize = { (float)target.get_width(), (float)target.get_height() };

		ImGui::GetIO();

		ImGui::NewFrame();
		
		ImGui::ShowDemoWindow();

		// calls EndFrame
		ImGui::Render();
		return ImGui::GetDrawData();
	}

	void renderer::render(const influx::renderer::scene& scene)
	{
		// updates the window target
		mp_window_target = influx::renderer::get_window_target(m_window_handle);

		influx::renderer::draw_scene(scene, *mp_scene_color_target);

		influx::renderer::draw_imgui(get_imgui_data(*mp_scene_color_target), *mp_scene_color_target);

		influx::renderer::copy_target(*mp_scene_color_target, *mp_window_target);

		influx::renderer::present_args present_args{};
		present_args.m_vsync = true;
		influx::renderer::present_swapchain(present_args);
	}

	void renderer::load_render_assets(content_manager* cont_man)
	{
		// load meshdata into renderer
		for (const auto& asset : cont_man->get_scenes())
		{
			const assets::scene_data::mesh& mesh = asset.second.m_meshes[0]; // gets the first mesh
			const std::string& name = asset.first;

			influx::renderer::mesh_data mesh_data{};
			for (size_t i = 0u; i < mesh.m_positions.size(); ++i)
			{
				mesh_data.m_vertices.push_back({});
				mesh_data.m_vertices.back().m_position = mesh.m_positions[i];
				// mesh_data.m_vertices.back().m_colour = mesh.m_colours[i];
				mesh_data.m_vertices.back().m_normal = mesh.m_normals[i];
				mesh_data.m_vertices.back().m_texcoords = mesh.m_uvs[i];
			}
			mesh_data.m_indices = mesh.m_indices;

			// load into the renderer
			influx::renderer::load(name, mesh_data);
		}

		// load shaders into renderer
		for (const auto& asset : cont_man->get_shaders())
		{
			const assets::shader_data& shader = asset.second;
			const string& name = asset.first;

			influx::renderer::shader_data shader_data{};
			shader_data.m_bytecode = shader.m_compile_result.m_bytecode;
			shader_data.m_type = shader.m_type;
			shader_data.m_reflection = shader.m_compile_result.m_reflection;

			influx::renderer::load(name, shader_data);
		}

		// load textures into renderer
		for (const auto& asset : cont_man->get_images())
		{
			const assets::image_data& image = asset.second;
			const string& name = asset.first;

			influx::renderer::texture_data tex_data{};
			tex_data.m_pixels = image.m_pixels;
			tex_data.m_width = image.m_dimensions.x;
			influx::renderer::load(name, tex_data);
		}
	}
}