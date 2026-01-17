#include "engine_pch.h"
#include "scene_editor.h"

// influx::engine
#include "input/input_manager.h"
#include "world/world.h"
#include "scene/scene.h"
#include "component/component.h"
#include "rendering/render_manager.h"
#include "content/content_manager.h"
#include "editor/editor_manager.h"

// influx::renderer
#include "influx_renderer/target.h"

// influx::platform
#include "influx_platform/window.h"

namespace influx::engine::editor
{
#pragma region editor
	class sceneview_editor final : public editor::editor_window
	{
	public:
		static math::transform3D m_camera_transform;

	public:
		virtual void on_run() override
		{
			const auto& matrix = m_camera_transform.get_matrix();
			const auto& position = m_camera_transform.get_position();
			const auto& eulers = m_camera_transform.get_rotation_eulers();
			const auto& scale = m_camera_transform.get_scale();

			ImGui::Text("[camera transform]");
			ImGui::Text("position: [%.2f, %.2f, %.2f]", position.x, position.y, position.z);
			ImGui::Text("rotation: [%.2f, %.2f, %.2f]", eulers.x, eulers.y, eulers.z);
			ImGui::Text("scale     [%.2f, %.2f, %.2f]", scale.x, scale.y, scale.z);
			ImGui::Text("");

			ImGui::Text("[camera transform matrix]");
			ImGui::Text("[%.2f, %.2f, %.2f, %.2f]", matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3]);
			ImGui::Text("[%.2f, %.2f, %.2f, %.2f]", matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3]);
			ImGui::Text("[%.2f, %.2f, %.2f, %.2f]", matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3]);
			ImGui::Text("[%.2f, %.2f, %.2f, %.2f]", matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3]);
		}
	};
	math::transform3D sceneview_editor::m_camera_transform = {};
#pragma endregion

	static transform_component* g_transform = nullptr;
	static camera_component* g_camera = nullptr;

	inline render_view& get_renderview()
	{
		render_manager& renderman = get_engine()->get_renderer();
		return renderman.get_renderview(e_render_view::scene_editor);
	}

	// places an object
	void scene_editor::on_edit_place()
	{
		world& world = get_engine()->get_world();
		auto entity = world.create_entity();
		{
			transform_component& transform = world.create_component<transform_component>(entity);
			transform.set_identity();

			mesh_component& mesh = world.create_component<mesh_component>(entity);
			const assets::mesh_id msh_id = assets::make_mesh_id("sphere_0");
			mesh.set_mesh_id(msh_id);

			render_component& render = world.create_component<render_component>(entity);
			render.set_view_visibility(e_view_visibility_flags::all);
		}
	}

	void scene_editor::on_edit_remove()
	{
	}

	void scene_editor::pick_scene(const input::mouse_position& position) const
	{
		if (g_camera && g_transform)
		{
			world& world = get_engine()->get_world();
			world::trace_result result = world.trace_main_scene(position.m_client_normalized).get();
			if (result.m_is_hit)
			{
				engine::log(e_log_category::info, "trace hit!");
			}
		}
	}

	scene_editor::scene_editor()
	{
		editor::editor_manager::static_window<sceneview_editor>("sceneview").set_name("sceneview");

		reset_camera();

		m_edit_radial.set_radius(60.0f);
		m_edit_radial.set_item("place", scene_editor::on_edit_place);
		m_edit_radial.set_item("remove", scene_editor::on_edit_remove);

		// temp: auto-place an entity
		world& world = get_engine()->get_world();
		const asset_manager& content = get_engine()->get_assetman();
		const assets::scene_id sc_id = assets::make_scene_id("cafeleblanc.fbx");
		cptr<assets::scene_asset> asset = content.find_asset<assets::e_asset_type::scene>(sc_id);
		const imp::scene_data& scene_data = asset->get_resource().m_imported_data;

		for (const auto& mesh : scene_data.get_meshes())
		{
			auto entity = world.create_entity();
			transform_component& transform = world.create_component<transform_component>(entity);
			transform.set_identity();
			mesh_component& mesh_comp = world.create_component<mesh_component>(entity);
			const string& mesh_name = scene_data.get_name(mesh);
			assets::mesh_id mesh_id = assets::make_mesh_id(mesh_name);
			mesh_comp.set_mesh_id(mesh_id);

			render_component& render = world.create_component<render_component>(entity);
			render.set_view_visibility(e_view_visibility_flags::all);
		}
	}

	scene_editor::~scene_editor()
	{

	}

	void scene_editor::update_inputs()
	{
		input_manager& inputman = get_engine()->get_input();

		// unreal camera controls
		float camera_sprint_input = 0.0f;
		math::float3 camera_move_input = {};
		if (inputman.is_down('E'))
		{
			camera_move_input += math::float3(0, -1, 0);
		}
		if (inputman.is_down('Q'))
		{
			camera_move_input += math::float3(0, 1, 0);
		}
		if (inputman.is_down('W'))
		{
			camera_move_input += math::float3(0, 0, 1);
		}
		if (inputman.is_down('A'))
		{
			camera_move_input += math::float3(-1, 0, 0);
		}
		if (inputman.is_down('S'))
		{
			camera_move_input += math::float3(0, 0, -1);
		}
		if (inputman.is_down('D'))
		{
			camera_move_input += math::float3(1, 0, 0);
		}
		if (inputman.is_down('R'))
		{
			reset_camera();
		}
		if (inputman.is_down(input::e_key::lshift))
		{
			camera_sprint_input = 1.0f;
		}

		math::float3 camera_world_delta_move = 
			camera_move_input.x * m_camera_transform.get_right() +
			camera_move_input.y * math::float3(0,1,0) +
			camera_move_input.z * m_camera_transform.get_forward();
		if (camera_world_delta_move.is_zero() == false) camera_world_delta_move.normalize();

		float camera_move_speed = 10.0f;
		camera_move_speed += camera_move_speed * camera_sprint_input * 1.5f;

		auto& time = get_engine()->get_time();
		const float delta_seconds = time.get_delta_seconds();
		camera_world_delta_move *= camera_move_speed * delta_seconds;
		
		m_camera_transform.move(camera_world_delta_move);
	}

	void scene_editor::reset_camera()
	{
		m_camera_transform = math::transform3D::identity();
		m_camera_transform.set_position(10, 10, 10);
		m_camera_transform.look_at({});
	}

	void scene_editor::on_imgui(ImGuiContext& ctx)
	{
		update_inputs();

		// cute edit radial
#if 0
		m_edit_radial.render(inputman.get_mouse_position_client());
		if (m_edit_radial.has_selection())
		{
			on_radial_select* ptr_ptr = m_edit_radial.get_selected();
			if (ptr_ptr) (*ptr_ptr)();
		}
#endif

		render_view& render_view = get_renderview();

		if (ImGui::Begin("scene"))
		{
			influx::camera& camera_settings = render_view.get_camera_settings();

			m_camera_transform.update_matrix();

			const auto current_size = ImGui::GetWindowSize();
			math::uint2 view_dimensions = { current_size.x, current_size.y };
			render_view.get_clear_colour() = { 0.1,0.1,0.1,1 };
			render_view.set_dimensions(view_dimensions);
			render_view.set_render_enabled(true);
			render_view.get_camera_transform() = m_camera_transform;
			camera_settings.set_aspect_ratio(current_size.x / current_size.y);
			camera_settings.set_fov(90.0f);
			camera_settings.set_is_orthographic(false);
			camera_settings.set_nearplane(0.001f);
			camera_settings.set_farplane(1000.0f);

			sceneview_editor::m_camera_transform = m_camera_transform.get_matrix();

			ImGui::Image(reinterpret_cast<ImTextureID>(&render_view.get_target()), 
				current_size);
			ImGui::End();
		}
		else
		{
			render_view.set_render_enabled(false);
		}
	}

	void scene_editor::on_mouse_down(input::e_mouse_button button, const input::mouse_position& position)
	{
		switch (button)
		{
		case input::e_mouse_button::left:
			pick_scene(position);
			break;
		case input::e_mouse_button::right:
			m_is_controlling_camera = true;
			break;
		}
	}

	void scene_editor::on_mouse_up(input::e_mouse_button button, const input::mouse_position& position)
	{
		switch (button)
		{
		case input::e_mouse_button::left:
			break;
		case input::e_mouse_button::right:
			m_is_controlling_camera = false;
			break;
		}
	}

	void scene_editor::on_mouse_move(const input::mouse_position& new_position)
	{
		if (m_is_controlling_camera)
		{
			auto& time = get_engine()->get_time();
			const float delta_seconds = time.get_delta_seconds();

			const math::float2 mouse_delta = new_position.m_client - m_last_mouse_position.m_client;
			if (mouse_delta.is_zero() == false)
			{
				m_camera_transform.rotate(
					mouse_delta.y * delta_seconds,
					mouse_delta.x * delta_seconds,
					0.0f);
			}
		}
		m_last_mouse_position = new_position;
	}
}