#include "engine_pch.h"
#include "editor_manager.h"

// influx::core
#include "core/log.h"

// influx::engine
#include "engine_private.h"
#include "file/engine_files.h"
#include "content/content_manager.h"
#include "world/world.h"
#include "influx_engine/scene/scene.h"
#include "content/content_manager.h"

// influx::platform
#include "influx_platform/window.h"

// influx::imgui
#include "influx_imgui/imgui_translation.h"
#include "influx_imgui/imgui_widgets.h"

// imgui
#include "imgui/imgui.h"

namespace influx::engine
{
#pragma region gamefile
	string make_game_directory_path(const string& game_name)
	{
		engine* engine = get_engine();
		const file& game_directory = engine->get_engine_directory(engine::e_directory::games);
		const string gamefile_subdir = game_directory.m_path_full + "/" + game_name + "/";
		return gamefile_subdir;
	}

	string make_gamefile_path(const string& game_name)
	{
		const string gamefile_subdir = make_game_directory_path(game_name);
		const string gamefile_ext = ".flx";

		const string gamefile_path =
			gamefile_subdir
			+ game_name
			+ gamefile_ext;

		return gamefile_path;
	}

	string make_scenefile_path(const string& game_name, const string& scene_name)
	{
		const string gamefile_subdir = make_game_directory_path(game_name);
		const string scenefile_ext = ".flx";

		const string scenefile_path =
			gamefile_subdir
			+ "/scenes/"
			+ scene_name
			+ scenefile_ext;

		return scenefile_path;
	}

	// loads file at /influx/games/'game_name'/
	bool load_gamefile(const string& game_name, file_game& out_gamefile)
	{
		const string gamefile_path = make_gamefile_path(game_name);
		if (file::exists(gamefile_path))
		{
			// open existing file
			out_gamefile.load(gamefile_path);
			return true;
		}

		return false;
	}

	// creates file at /influx/games/'game_name'/
	bool create_gamefile(const string& game_name, file_game& out_gamefile)
	{
		const string gamefile_path = make_gamefile_path(game_name);
		if (file::exists(gamefile_path))
		{
			// return false;
		}
		
		// new file
		out_gamefile = {};
		out_gamefile.m_name = game_name;
		out_gamefile.m_id = 0u;
		out_gamefile.save(gamefile_path);
		return true;
	}
#pragma endregion

	editor_manager::editor_manager(editor_module* editor)
		: m_editor{ editor }
		, m_popup_radial{}
	{
		// defaults
		m_editor_toggle.force_set(true);
		m_engine_toggle.force_set(true);
		m_content_toggle.force_set(false);

		initialize_inputs();

		// temp
		set_target_game(*get_engine(), "influx_game");
	}

	result editor_manager::update_imgui(ImGuiContext& ctx)
	{
		update_context();
		update_inputs();
		update_main_editor();

		// user-module after main editor
		m_editor->on_imgui(ctx);

		return {};
	}

	result editor_manager::update_inputs()
	{
		// ctrl + space: engine + content
		if (m_keybinds.is_dualbind_new(input::e_key::lctrl, input::e_key::space))
		{
			m_content_toggle = !m_content_toggle;
			m_engine_toggle = !m_engine_toggle;
		}

		if (m_keybinds.is_dualbind_new(input::e_key::lctrl, input::e_key::lalt))
		{
			m_editor_toggle = !m_editor_toggle;
		}

		return {};
	}

	result editor_manager::update_context()
	{
		engine* engine = get_engine();
		influx_assert(engine);

		const platform::window* window = engine->get_window();
		influx_assert(window);

		const math::vectoru2& window_dimensions = window->get_dimensions(platform::window::e_space::client);
		ImGui::GetIO().DisplaySize = { (float)window_dimensions.x, (float)window_dimensions.y };

		return {};
	}

	result editor_manager::update_main_editor()
	{
		engine* engine = get_engine();
		influx_assert(engine);

		world* world = engine->get_world();
		influx_assert(world);

		content_manager* content = engine->get_content();
		influx_assert(content);

		if (!m_editor_toggle)
		{
			return {};
		}

		// "main menu"
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("file"))
			{
				if (ImGui::Button("new game"))
				{
					create_gamefile("influx_game", m_current_gamefile);
				}

				if (ImGui::Button("load game"))
				{
					load_gamefile("influx_game", m_current_gamefile);
				}

				if (ImGui::Button("new scene"))
				{
					// new empty scene
					scene* new_scene = scene::make_empty_scene();

					// save to file
					const string& scenefile_path = make_scenefile_path("influx_game", "scene_main");
					scene::save_to_file(new_scene, scenefile_path);

					// load into world
					world->load_scene(new_scene);
				}

				if (ImGui::Button("load scene"))
				{
					// load from file
					const string& scenefile_path = make_scenefile_path("influx_game", "scene_main");
					scene* scene = scene::load_from_file(scenefile_path);

					// load into world
					world->load_scene(scene);
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("edit"))
			{
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		// "influx engine"
		if (m_engine_toggle)
		{
			if (ImGui::Begin("influx engine"))
			{
				imgui::scoped_style_var minsize(ImGuiStyleVar_WindowMinSize, ImVec2(1000, 1000));
				ImGui::Text("fps: %f", engine->get_fps());
			}
			ImGui::End();
		}

		// "game:content"
		if (m_content_toggle)
		{
			if (ImGui::Begin(has_game() ? (get_game_name() + ":content").c_str() : "content"))
			{
				// "scene:filepath"
				for (const auto& pair : content->get_scenes())
					if (pair.second.is_loaded()) 
						ImGui::Text("scene:%s - ms:%f", pair.first.c_str(), pair.second.get_load_ms());
				ImGui::Text("--");
				// "texture:filepath"
				for (const auto& pair : content->get_images())
					if (pair.second.is_loaded())
						ImGui::Text("texture:%s - ms:%f", pair.first.c_str(), pair.second.get_load_ms());
				ImGui::Text("--");
				// "shader:filepath"
				for (const auto& pair : content->get_shaders())
					if (pair.second.is_loaded())
						ImGui::Text("shader:%s - ms:%f", pair.first.c_str(), pair.second.get_load_ms());
				ImGui::Text("--");
			}
			ImGui::End();
		}

		// radial menu
		update_radial_menu();

		return {};
	}

	result editor_manager::update_radial_menu()
	{
		// temp: size animation
		const float max_radius = 50.0f;
		const float seconds = get_engine()->get_time().m_time_seconds;
		const float anim_speed = 5.0f;
		const float radius = math::pingpong(seconds * anim_speed, max_radius * 0.95f, max_radius);

		m_popup_radial.set_id("##piepopup");
		m_popup_radial.set_items({ "new", "old" });
		m_popup_radial.set_radius(radius);
		m_popup_radial.render(m_mousepos);

		if (m_popup_radial.has_selection())
			logn("selected: {}", m_popup_radial.get_selected());

		return {};
	}

	result editor_manager::set_target_game(engine& engine, const string& gamename)
	{
		if (has_game() && get_game_name() == gamename)
		{
			return {};
		}

		// try load, if fail, create game
#if 0
		if (!load_gamefile(gamename, m_current_gamefile))
		{
			create_gamefile(gamename, m_current_gamefile);
		}
#else
		create_gamefile(gamename, m_current_gamefile);
#endif

		engine.get_content()->load_game_assets(gamename, &engine);
		return {};
	}

	result editor_manager::on_keydown(input::e_key key)
	{
		m_keybinds.set(key, true);
		return {};
	}

	result editor_manager::on_keyup(input::e_key key)
	{
		m_keybinds.set(key, false);
		return {};
	}

	result editor_manager::on_ascii_down(char ascii)
	{
		m_keybinds.set(ascii, true);
		return {};
	}

	result editor_manager::on_ascii_up(char ascii)
	{
		m_keybinds.set(ascii, false);
		return {};
	}

	result editor_manager::on_mouse_down(input::e_mouse_button button, const input::mouse_position& position)
	{
		switch (button)
		{
		case input::e_mouse_button::right: 
			m_popup_radial.set_visible(true);
			m_popup_radial.set_position(position.m_client);
			break;
		}

		return {};
	}

	result editor_manager::on_mouse_up(input::e_mouse_button button, const input::mouse_position& position)
	{
		switch (button)
		{
		case input::e_mouse_button::right:
			m_popup_radial.set_visible(false);
			break;
		}

		return {};
	}

	result editor_manager::on_mouse_move(const input::mouse_position& position)
	{
		m_mousepos = position.m_client;
		return {};
	}

	bool editor_manager::has_game() const
	{
		return m_current_gamefile.m_name != "";
	}

	string editor_manager::get_game_name() const
	{
		if (has_game())
		{
			return m_current_gamefile.m_name;
		}

		return "";
	}

	result editor_manager::initialize_inputs()
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

		return {};
	}
}