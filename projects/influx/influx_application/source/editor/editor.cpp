#include "app_pch.h"
#include "editor.h"

// influx::input
#include "influx_input.h"

// imgui
#include "imgui/imgui.h"

namespace influx::application
{
	editor::editor()
	{
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

		set_window_dimensions({ 1280, 720 });
	}

	void editor::set_window_dimensions(const math::vectorf2& dimensions)
	{
		ImGui::GetIO().DisplaySize = { (float)dimensions.x, (float)dimensions.y };
	}

	void editor::update()
	{
		
	}

	ImDrawData* editor::get_imgui_drawdata()
	{
		ImGui::NewFrame();

		for (const auto& callback : m_callbacks)
		{
			callback();
		}

		ImGui::ShowDemoWindow();

		ImGui::Render(); // EndFrame
		return ImGui::GetDrawData();
	}

	void editor::subscribe(const imgui_callback& callback)
	{
		m_callbacks.push_back(callback);
	}

	void editor::unsubscribe(const imgui_callback& callback)
	{
		// m_callbacks.remove_if([&callback](const imgui_callback& list_clb) { return callback. == list_clb;  });
	}

	void editor::draw_transform(const math::transform3D& transform, const string& tag)
	{
		const math::vectorf3& position = transform.get_position();
		const math::rotation& rotation = transform.get_rotation();
		const math::vectorf3& scale = transform.get_scale();

		ImGui::Text(tag.c_str());
		ImGui::Text("Position | x:%f | y:%f | z:%f |", position.x, position.y, position.z);
		ImGui::Text("Rotation | x:%f | y:%f | z:%f |", 0.0f, 0.0f, 0.0f);
		ImGui::Text("Scale    | x:%f | y:%f | z:%f |", scale.x, scale.y, scale.z);
	}

	void editor::draw_vector3(const math::vectorf3& vec3, const string& tag)
	{
		ImGui::Text((tag + " | x:%f | y:%f | z:%f |").c_str(), vec3.x, vec3.y, vec3.z);
	}

	void editor::draw_mat4x4(const math::matrix4x4f& mat4x4, const string& tag)
	{
		ImGui::Text(tag.c_str());
		for (size_t i = 0u; i < 4u; ++i)
		{
			const auto& row = mat4x4.get_row(i);
			ImGui::Text("| %f | %f | %f | %f |", row.x, row.y, row.z, row.w);
		}
	}

	void editor::draw_camera(const scene::camera& camera, const string& tag)
	{
		ImGui::Text(tag.c_str());
		ImGui::Text("Fov: %f", camera.get_fov());
		ImGui::Text("Near/Far: %f/%f", camera.get_nearplane(), camera.get_farplane());
	}
}