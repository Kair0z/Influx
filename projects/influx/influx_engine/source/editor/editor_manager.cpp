#include "engine_pch.h"
#include "editor_manager.h"

// influx::core
#include "core/log.h"

// influx::engine
#include "engine.h"
#include "file/engine_files.h"
#include "content/content_manager.h"
#include "world/world.h"
#include "scene/scene.h"
#include "game/game_manager.h"
#include "rendering/render_manager.h"

// influx::platform
#include "influx_platform/window.h"
#include "influx_platform/library.h"

// influx::imgui
#include "influx_imgui/imgui_translation.h"
#include "influx_imgui/imgui_widgets.h"

// imgui
#include "imgui/imgui.h"

namespace influx::engine
{
#pragma region uis
	class game_manager_ui final : public editor_window
	{
	public:
		virtual void on_run() override
		{
			if (!m_is_running)
			{
				if (ImGui::Button("play"))
				{
					m_is_running = true;
					get_engine()->get_game().start();
				}
			}
			else
			{
				if (ImGui::Button("end"))
				{
					m_is_running = false;
					get_engine()->get_game().end();
				}
			}
		}

	private:
		bool m_is_running = false;
	};

	class content_ui final : public editor_window
	{
	public:
		virtual void on_run() override
		{
			static content_manager& content = get_engine()->get_content();
			static render_manager& renderer = get_engine()->get_renderer();

			set_name("engine:content");

			if (ImGui::Button("recomp_shaders"))
			{
				for (auto& pair : content.touch_shaders())
				{
					pair.second.reload();
				}
			}

			if (ImGui::BeginTabBar("content"))
			{
				if (ImGui::BeginTabItem("scenes"))
				{
					// "scene:filepath"
					for (const auto& pair : content.get_scenes())
					{
						const string& name = pair.first;
						const scene_asset& scene_asset = pair.second;

						if (scene_asset.is_loaded() && scene_asset.is_engine())
						{
							if (ImGui::TreeNode(name.c_str(), "scene: %s - ms : % f", name.c_str(), scene_asset.get_load_ms()))
							{
								for (uint32 i = 0u; i < scene_asset.get_resource().get_num_meshes(); ++i)
								{
									const string& mesh_name = name + "_" + to_string(i);
									ImGui::Text(mesh_name.c_str());
								}
								ImGui::TreePop();
							}
						}
					}
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("textures"))
				{
					const float size = 50.0f;

					if (ImGui::BeginTable("ed_texture_grid", 4u))
					{
						for (const auto& pair : content.get_images())
						{
							if (pair.second.is_loaded() && pair.second.is_engine())
							{
								ImGui::TableNextColumn();

								const string& name = pair.first;
								// ImGui::Image(reinterpret_cast<ImTextureID>(renderer.get_loaded_texture_id(name)), { size, size });

								const image_asset& image = pair.second;
								const math::vectori2& image_dims = image.m_resource.m_dimensions;
								ImGui::TextWrapped("%s", name.c_str());
							}
						}
						ImGui::EndTable();
					}

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("shaders"))
				{
					// "shader:filepath"
					for (const auto& pair : content.get_shaders())
						if (pair.second.is_loaded() && pair.second.is_engine())
							ImGui::Text("shader:%s - ms:%f", pair.first.c_str(), pair.second.get_load_ms());
					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}
		}
	};

	class fps_ui final : public editor_window
	{
	public:
		virtual void on_run() override
		{
			set_name("influx engine");
			imgui::scoped_style_var minsize(ImGuiStyleVar_WindowMinSize, ImVec2(1000, 1000));
			ImGui::Text("fps: %f", get_engine()->get_fps());
		}
	};
#pragma endregion

	// all static windows of the engine
	umap<string, editor_window*> editor_manager::m_static_windows{};

	editor_manager::editor_manager()
		: m_static_windows_radial{}
		, m_edit_radial{}
	{
		initialize_inputs();
		load_editor();
	}

	editor_manager::~editor_manager()
	{
		save_editor();
	}

	void editor_manager::on_imgui(ImGuiContext& ctx)
	{
		update_context();
		update_inputs();
		update_background_dockspace();
		update_mainmenu();
		update_static_windows();
		update_edit_radial();
	}

	void editor_manager::update_inputs()
	{
		// ctrl + space: engine + content
		if (m_keybinds.is_dualbind_new(input::e_key::lctrl, input::e_key::space))
		{
			m_content_toggle = !m_content_toggle;
			m_engine_toggle = !m_engine_toggle;
		}

		// 
		if (m_keybinds.is_dualbind_new(input::e_key::lctrl, input::e_key::lalt))
		{
			m_editor_toggle = !m_editor_toggle;
		}
	}

	void editor_manager::update_context()
	{
		engine& engine = *get_engine();
		const math::vectoru2& window_dimensions = engine.get_window().get_dimensions(platform::window::e_space::client);
		ImGui::GetIO().DisplaySize = { (float)window_dimensions.x, (float)window_dimensions.y };
	}

	void editor_manager::update_mainmenu()
	{
		m_is_mainmenu_active = true;
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("project"))
			{
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void editor_manager::update_background_dockspace()
	{
		// Get the current viewport
		ImGuiViewport* viewport = ImGui::GetMainViewport();

		const float mainmenu_height = get_mainmenu_height();

		// Set up a window that spans the entire viewport
		ImVec2 windowpos = viewport->Pos; windowpos.y += mainmenu_height;
		ImVec2 windowsize = viewport->Size; windowsize.y -= mainmenu_height;
		ImGui::SetNextWindowPos(windowpos);
		ImGui::SetNextWindowSize(windowsize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::SetNextWindowBgAlpha(0.0f);

		// Set window flags to make it invisible and non-interactive
		ImGuiWindowFlags window_flags = 
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse | 
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove | 
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus |
			ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("InvisibleDockSpaceX", nullptr, window_flags);
		ImGui::PopStyleVar(3);

		// Create the dock space
		ImGuiID dockspace_id = ImGui::GetID("InvisibleDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

		ImGui::End();
	}

	void editor_manager::update_static_windows()
	{
		static_window<game_manager_ui>("game");
		static_window<fps_ui>("fps");
		static_window<content_ui>("content");

		const float max_radius = 50.0f;
		const float seconds = get_engine()->get_time().get_time_seconds();
		const float anim_speed = 5.0f;
		const float radius = math::pingpong(seconds * anim_speed, max_radius * 0.95f, max_radius);

		m_static_windows_radial.set_id("##piepopup");
		m_static_windows_radial.set_radius(radius);
		m_static_windows_radial.render(m_mousepos);

		for (auto& pair : m_static_windows)
		{
			if (!pair.first.empty() && pair.second != nullptr)
			{
				m_static_windows_radial.set_item(pair.first, pair.second);
			}
		}

		if (m_static_windows_radial.has_selection())
		{
			editor_window* selected = *m_static_windows_radial.get_selected();
			selected->toggle();
		}

		for (auto& pair : m_static_windows)
		{
			if (pair.second && pair.second->is_visible())
			{
				pair.second->run({});
			}
		}
	}

	void editor_manager::update_edit_radial()
	{
		static entity custom_entity{};

		const float max_radius = 50.0f;
		const float seconds = get_engine()->get_time().get_time_seconds();
		const float anim_speed = 5.0f;
		const float radius = math::pingpong(seconds * anim_speed, max_radius * 0.95f, max_radius);

		static world& world = get_engine()->get_world();
		m_edit_radial.set_item("create", []()
		{
			custom_entity = world.create_entity();
			
			transform_component& ent_transform = world.create_component<transform_component>(custom_entity);
			ent_transform.set_position({});
			ent_transform.set_scale(0.001f);
			ent_transform.update_matrix();

			// mesh
			mesh_component& ent_mesh = world.create_component<mesh_component>(custom_entity);
			ent_mesh.set_mesh_name("esphere");
			ent_mesh.set_use_normalized_scale(true); // scales to bounding sphere
			ent_mesh.set_invert_normals(false);

			// material
			material_component& ent_mat = world.create_component<material_component>(custom_entity);
			ent_mat.set_texture(e_texture_semantic::basecolor, "lego");
			ent_mat.set_texture(e_texture_semantic::normals, "");
			ent_mat.set_texture(e_texture_semantic::roughness, "");
		});

		m_edit_radial.set_item("destroy", []()
		{
			world.destroy_entity(custom_entity);
		});

		m_edit_radial.set_id("##piepopup_edit");
		m_edit_radial.set_radius(radius);
		m_edit_radial.render(m_mousepos);
		if (m_edit_radial.has_selection())
		{
			m_edit_radial.get_selected()->operator()();
		}
	}

	void editor_manager::on_keydown(input::e_key key)
	{
		m_keybinds.set(key, true);
	}

	void editor_manager::on_keyup(input::e_key key)
	{
		m_keybinds.set(key, false);
	}

	void editor_manager::on_ascii_down(char ascii)
	{
		m_keybinds.set(ascii, true);
	}

	void editor_manager::on_ascii_up(char ascii)
	{
		m_keybinds.set(ascii, false);
	}

	void editor_manager::on_mouse_down(input::e_mouse_button button, const input::mouse_position& position)
	{
		switch (button)
		{
		case input::e_mouse_button::right: 
			m_edit_radial.set_visible(true);
			m_edit_radial.set_position(position.m_client);
			break;

		case input::e_mouse_button::left:
			
			world& world = get_engine()->get_world();

			// get camera matrices
			const math::matrix4x4f projection = world.get_main_projection_matrix();
			const math::matrix4x4f view = world.get_main_viewmatrix();
			const math::float3 camera_pos = world.get_main_cameraposition();

			// convert pixel to ndc space
			const math::float2 clientpos = position.m_client;
			math::float3 mouse_ndc =
			{
				(2.0f * clientpos.x / 1280.0f) - 1.0f,
				1.0f - (2.0f * clientpos.y / 720.0f),
				-1.0f
			};

			mouse_ndc.x = -mouse_ndc.x;
			mouse_ndc.y = -mouse_ndc.y;

			// unproject ndc -> view
			const math::float3 raypos_ndc = math::float4(mouse_ndc.x, mouse_ndc.y, mouse_ndc.z);
			math::float3 raypos_view = projection.inverted() * raypos_ndc;
			raypos_view.z = -1.0f;

			// unview view -> world
			math::float4 raypoint_world = view.inverted() * raypos_view;
			
			// make the ray
			math::ray ray_from_eye{};
			ray_from_eye.m_direction = -(raypoint_world.get_xyz() - camera_pos).normalized();
			ray_from_eye.m_origin = camera_pos;
			ray_from_eye.m_min = 0.0f;
			ray_from_eye.m_max = FLT_MAX;

			// trace the world with the ray
			world::trace_result result{};
			world.trace(ray_from_eye, result);
			break;
		}
	}

	void editor_manager::on_mouse_up(input::e_mouse_button button, const input::mouse_position& position)
	{
		switch (button)
		{
		case input::e_mouse_button::right:
			m_edit_radial.set_visible(false);
			break;
		}
	}

	void editor_manager::on_mouse_move(const input::mouse_position& position)
	{
		m_mousepos = position.m_client;
	}

	bool editor_manager::has_project() const
	{
		return m_projectfile.m_name != "";
	}

	string editor_manager::get_projectname() const
	{
		if (has_project())
		{
			return m_projectfile.m_name;
		}

		return "";
	}

	string editor_manager::get_editor_filepath() const
	{
		return get_engine_directory(engine_directory::editor).m_path_full + "editor.flx";
	}

	void editor_manager::save_editor()
	{
		m_editorfile.save(get_editor_filepath());
	}

	void editor_manager::load_editor()
	{
		const string& filepath = get_editor_filepath();
		if (file::exists(filepath))
			m_editorfile.load(filepath);
	}

	files::editorfile& editor_manager::get_editorfile()
	{
		return m_editorfile;
	}

	float editor_manager::get_mainmenu_height() const
	{
		return m_is_mainmenu_active ? ImGui::GetFrameHeight() : 0.0f;
	}

	void editor_manager::initialize_inputs()
	{
		input::subscribe_keydown([this](input::e_key key) { on_keydown(key); });
		input::subscribe_keyup([this](input::e_key key) { on_keyup(key); });
		input::subscribe_asciidown([this](char ascii) { on_ascii_down(ascii); });
		input::subscribe_asciiup([this](char ascii) { on_ascii_up(ascii); });
		input::subscribe_mousemove([this](const input::mouse_position& position)
		{
			on_mouse_move(position);
		});
		input::subscribe_mousedown([this](input::e_mouse_button button, const input::mouse_position& position)
		{
			on_mouse_down(button, position);
		});
		input::subscribe_mouseup([this](input::e_mouse_button button, const input::mouse_position& position)
		{
			on_mouse_up(button, position);
		});
	}
}