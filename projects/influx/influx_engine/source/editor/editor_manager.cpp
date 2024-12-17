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
#include "content/content_manager.h"

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
#pragma region gamefile
	string make_game_directory_path(const string& game_name)
	{
		engine* engine = get_engine();
		const file& game_directory = get_engine_directory(engine_directory::games);
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
	bool load_gamefile(const string& game_name, files::projectfile& out_gamefile)
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
	bool create_gamefile(const string& game_name, files::projectfile& out_gamefile)
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

	static math::transform3D g_inspect_transform{};
	math::transform3D& editor_manager::get_inspect_transform()
	{
		return g_inspect_transform;
	}

	editor_manager::editor_manager(editor_module* editor)
		: m_editor{ editor }
		, m_popup_radial{}
	{
		// defaults
		m_editor_toggle.force_set(true);
		m_engine_toggle.force_set(true);
		m_fps_toggle.force_set(true);
		m_content_toggle.force_set(true);

		initialize_inputs();

		// temp
		set_target_game(*get_engine(), "influx_game");
	}

	result<> editor_manager::update_imgui(ImGuiContext& ctx)
	{
		update_context();
		update_inputs();
		//update_background_dockspace();
		update_main_editor();
		return {};
	}

	result<> editor_manager::update_inputs()
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

	result<> editor_manager::update_context()
	{
		result<> res{};

		engine* engine = get_engine();
		influx_assert(engine);

		if (cptr<platform::window> window = res.append_and_get(engine->get_window()))
		{
			const math::vectoru2& window_dimensions = window->get_dimensions(platform::window::e_space::client);
			ImGui::GetIO().DisplaySize = { (float)window_dimensions.x, (float)window_dimensions.y };
		}

		return res;
	}

	result<> editor_manager::update_main_editor()
	{
		result<> result{};

		engine* engine = get_engine();
		influx_assert(engine);

		auto res_world = engine->get_world();
		influx_assert(res_world);

		auto res_content = engine->get_content();
		influx_assert(res_content);

		if (!m_editor_toggle)
		{
			return {};
		}

		// "main menu"
		update_mainmenu();

		// "fps"
		{
			m_fps_window.set_visible(m_fps_toggle);
			m_fps_window.set_name("influx engine");
			m_fps_window.run([&engine]()
			{
				imgui::scoped_style_var minsize(ImGuiStyleVar_WindowMinSize, ImVec2(1000, 1000));
				ImGui::Text("fps: %f", engine->get_fps());
			});
		}

		// "game:content"
		{
			const math::float2 window_size = { 200.0f, 200.0f };
			const float t = math::pingpong(engine->get_time().get_time_seconds(), 0.0f, 1.0f);
			const math::float2 animated_pos = math::lerp<math::float2>(t, { 0, 0 }, { 1280 - window_size.x, 720 - window_size.y });
			const string name = has_project() ? (get_projectname().get() + ":content") : "content";

			m_content_window.set_visible(m_content_toggle);
			// m_content_window.set_position(animated_pos);
			m_content_window.set_size(window_size);
			m_content_window.set_name(name);
			m_content_window.run([&res_content]()
			{
				// "scene:filepath"
				for (const auto& pair : res_content->get_scenes())
					if (pair.second.is_loaded() && pair.second.is_game())
						ImGui::Text("scene:%s - ms:%f", pair.first.c_str(), pair.second.get_load_ms());
				ImGui::Text("--");
				// "texture:filepath"
				for (const auto& pair : res_content->get_images())
					if (pair.second.is_loaded() && pair.second.is_game())
						ImGui::Text("texture:%s - ms:%f", pair.first.c_str(), pair.second.get_load_ms());
				ImGui::Text("--");
				// "shader:filepath"
				for (const auto& pair : res_content->get_shaders())
					if (pair.second.is_loaded() && pair.second.is_game())
						ImGui::Text("shader:%s - ms:%f", pair.first.c_str(), pair.second.get_load_ms());
				ImGui::Text("--");
			});

			m_engine_content_window.set_name("engine:content");
			m_engine_content_window.run([&res_content]()
			{
				// "scene:filepath"
				for (const auto& pair : res_content->get_scenes())
					if (pair.second.is_loaded() && pair.second.is_engine())
						ImGui::Text("scene:%s - ms:%f", pair.first.c_str(), pair.second.get_load_ms());
				ImGui::Text("--");
				// "texture:filepath"
				for (const auto& pair : res_content->get_images())
					if (pair.second.is_loaded() && pair.second.is_engine())
					{
						const image_asset& image = pair.second;
						const math::vectori2& image_dims = image.m_resource.m_dimensions;
						ImGui::Text("texture:%s - ms:%f[%ix%i]", pair.first.c_str(), image.get_load_ms(), 
							image_dims.x, image_dims.y);
					}
						
				ImGui::Text("--");
				// "shader:filepath"
				for (const auto& pair : res_content->get_shaders())
					if (pair.second.is_loaded() && pair.second.is_engine())
						ImGui::Text("shader:%s - ms:%f", pair.first.c_str(), pair.second.get_load_ms());
				ImGui::Text("--");
			});
		}

		static editor_window transform_window{};
		transform_window.set_visible(true);
		transform_window.set_name("transform");
		transform_window.run([]()
		{
			imgui::transform3D("camera transform", g_inspect_transform);
		});

		// radial menu
		update_radial_menu();

		return {};
	}

	result<> editor_manager::update_mainmenu()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("file"))
			{
				if (ImGui::Button("new game"))
				{
					create_gamefile("influx_game", m_projectfile);
				}

				if (ImGui::Button("load game"))
				{
					load_gamefile("influx_game", m_projectfile);
				}

				if (ImGui::Button("new scene"))
				{
					// new empty scene
					//scene* new_scene = scene::make_empty_scene();

					// save to file
					// const string& scenefile_path = make_scenefile_path("influx_game", "scene_main");
					// scene::save_to_file(new_scene, scenefile_path);

					// load into world
					// world->load_scene(new_scene);
				}

				if (ImGui::Button("load scene"))
				{
					// load from file
					// const string& scenefile_path = make_scenefile_path("influx_game", "scene_main");
					// scene* scene = scene::load_from_file(scenefile_path);

					// load into world
					// world->load_scene(scene);
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("edit"))
			{
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		return {};
	}

	result<> editor_manager::update_radial_menu()
	{
		// temp: size animation
		const float max_radius = 50.0f;
		const float seconds = get_engine()->get_time().get_time_seconds();
		const float anim_speed = 5.0f;
		const float radius = math::pingpong(seconds * anim_speed, max_radius * 0.95f, max_radius);

		m_popup_radial.set_id("##piepopup");
		m_popup_radial.set_items({ "new", "old", "load"});
		m_popup_radial.set_radius(radius);
		m_popup_radial.render(m_mousepos);

		if (m_popup_radial.has_selection())
		{
			const char* selected = m_popup_radial.get_selected();
			if (selected == "load")
			{
				string dll_dir = "D:/Git/Influx/bin/debug-windows-x86_64/influx_game/";
				string dll_path = dll_dir + "influx_game.dll";
				
				// load dll
				static platform::library* lib = nullptr;
				if (lib == nullptr)
				{
					lib = platform::library::load(dll_path);
				}

				for (string func : lib->get_functions())
				{
					logn("influx_game:{}", func);
				}

				lib->call("foo");
			}
		}

		return {};
	}

	result<> editor_manager::update_background_dockspace()
	{
		// Get the current viewport
		ImGuiViewport* viewport = ImGui::GetMainViewport();

		// Set up a window that spans the entire viewport
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);

		// Set window flags to make it invisible and non-interactive
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus;

		window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("InvisibleDockSpace", nullptr, window_flags);
		ImGui::PopStyleVar(3);

		// Create the dock space
		ImGuiID dockspace_id = ImGui::GetID("InvisibleDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

		ImGui::End();

		return {};
	}

	result<> editor_manager::set_target_game(engine& engine, const string& gamename)
	{
		if (has_project() && get_projectname().get() == gamename)
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
		create_gamefile(gamename, m_projectfile);
#endif

		engine.get_content()->load_game_assets(gamename, &engine);
		return {};
	}

	result<> editor_manager::on_keydown(input::e_key key)
	{
		m_keybinds.set(key, true);
		return {};
	}

	result<> editor_manager::on_keyup(input::e_key key)
	{
		m_keybinds.set(key, false);
		return {};
	}

	result<> editor_manager::on_ascii_down(char ascii)
	{
		m_keybinds.set(ascii, true);
		return {};
	}

	result<> editor_manager::on_ascii_up(char ascii)
	{
		m_keybinds.set(ascii, false);
		return {};
	}

	result<> editor_manager::on_mouse_down(input::e_mouse_button button, const input::mouse_position& position)
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

	result<> editor_manager::on_mouse_up(input::e_mouse_button button, const input::mouse_position& position)
	{
		switch (button)
		{
		case input::e_mouse_button::right:
			m_popup_radial.set_visible(false);
			break;
		}

		return {};
	}

	result<> editor_manager::on_mouse_move(const input::mouse_position& position)
	{
		m_mousepos = position.m_client;
		return {};
	}

	bool editor_manager::has_project() const
	{
		return m_projectfile.m_name != "";
	}

	result<string> editor_manager::get_projectname() const
	{
		if (has_project())
		{
			return m_projectfile.m_name;
		}

		return "";
	}

	result<> editor_manager::initialize_inputs()
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