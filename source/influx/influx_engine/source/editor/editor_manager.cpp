#include "engine_pch.h"
#include "editor_manager.h"

// influx::core
#include "core/log.h"

// influx::engine
#include "engine.h"
#include "file/engine_files.h"
#include "content/content_manager.h"
#include "world/world.h"
#include "game/game_manager.h"
#include "input/input_manager.h"

// influx::platform
#include "influx_platform/window.h"

// influx::imgui
#include "influx_imgui/imgui_translation.h"
#include "influx_imgui/imgui_widgets.h"

// imgui
#include "imgui/imgui.h"

namespace influx::engine::editor
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
	{
		load_editor();
	}

	editor_manager::~editor_manager()
	{
		save_editor();
	}

	void editor_manager::on_imgui(ImGuiContext& ctx)
	{
		update_inputs();
		update_context();
		update_background_dockspace();
		update_mainmenu();
		update_static_windows();
		
		m_scene_editor.on_imgui(ctx);
	}

	void editor_manager::update_inputs()
	{
		input_manager& inputman = get_engine()->get_input();

		const input::mouse_position mouse_position = inputman.get_mouse_position();
		m_mousepos = mouse_position.m_client;

		// mouse updates
		const buttonstate& lm_button = inputman.get_mousebutton_state(input::e_mouse_button::left);
		if (lm_button.is_firstframe_down())
		{
			m_scene_editor.on_mouse_down(input::e_mouse_button::left, mouse_position);
		}
		if (lm_button.is_firstframe_up())
		{
			m_scene_editor.on_mouse_up(input::e_mouse_button::left, mouse_position);
		}
		const buttonstate& rm_button = inputman.get_mousebutton_state(input::e_mouse_button::right);
		if (rm_button.is_firstframe_down())
		{
			m_scene_editor.on_mouse_up(input::e_mouse_button::right, mouse_position);
		}
		if (rm_button.is_firstframe_up())
		{
			m_scene_editor.on_mouse_up(input::e_mouse_button::right, mouse_position);
		}

		// 
		const buttonstate& lalt_button = inputman.get_keystate(input::e_key::lalt);
		const buttonstate& space_button = inputman.get_keystate(input::e_key::space);
		if (lalt_button.m_is_down && space_button.m_is_down)
		{
			// alt + space
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
				if (ImGui::Button("import fbx"))
				{
					platform::file_dialog_result result = platform::platform::open_file_dialog("");
					if (result.m_has_selected)
					{
						static content_manager& content = get_engine()->get_content();
						content.load(result.m_selection);
					}
				}

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
		
		for (auto& pair : m_static_windows)
		{
			if (pair.second && pair.second->is_visible())
			{
				pair.second->run({});
			}
		}
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
}