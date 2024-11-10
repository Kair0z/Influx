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

// imgui
#include "imgui/imgui.h"

namespace influx::imgui
{
	struct scoped_style_var
	{
	public:
		explicit scoped_style_var(ImGuiStyleVar style, const float& value)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, value);
		}

		explicit scoped_style_var(ImGuiStyleVar style, const ImVec2& value)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, value);
		}

		~scoped_style_var()
		{
			ImGui::PopStyleVar();
		}
	};
}

namespace influx::engine
{
#pragma region gamefile
	string make_gamefile_path(const string& game_name)
	{
		engine* engine = get_engine();
		const file& game_directory = engine->get_engine_directory(engine::e_directory::games);
		const string gamefile_subdir = "/" + game_name + "/";
		const string gamefile_ext = ".flx";

		const string gamefile_path =
			game_directory.m_path_full
			+ gamefile_subdir
			+ game_name
			+ gamefile_ext;

		return gamefile_path;
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
	{
		m_editor_toggle.force_set(true);

		set_target_game(*get_engine(), "influx_game");

		initialize_inputs();
	}

	void editor_manager::on_imgui(ImGuiContext& ctx)
	{
		engine* engine = get_engine();
		influx_assert(engine);

		process_inputs();

		if (!m_editor_toggle)
		{
			return;
		}

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

				if (ImGui::Button("load scene"))
				{
					// load the main scene
					scene* new_scene = scene::load_from_file("");
					engine->get_world()->load_scene(new_scene);
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("edit"))
			{
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		if (m_engine_toggle)
		{
			if (ImGui::Begin("influx engine"))
			{
				imgui::scoped_style_var minsize(ImGuiStyleVar_WindowMinSize, ImVec2(1000, 1000));
				ImGui::Text("fps: %f", engine->get_fps());
			}
			ImGui::End();
		}

		if (m_content_toggle)
		{
			if (ImGui::Begin(has_game() ? (get_game_name() + ":content").c_str() : "content"))
			{

			}
			ImGui::End();
		}

		// user-module after main editor
		m_editor->on_imgui(ctx);
	}

	void editor_manager::process_inputs()
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
	}

	void editor_manager::set_target_game(engine& engine, const string& gamename)
	{
		if (has_game() && get_game_name() == gamename)
		{
			return;
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

	void editor_manager::initialize_inputs()
	{
		input::subscribe_keydown([this](input::e_key key)
		{
			on_keydown(key);
		});

		input::subscribe_keyup([this](input::e_key key)
		{
			on_keyup(key);
		});

		input::subscribe_asciidown([this](char ascii)
		{
			on_ascii_down(ascii);
		});

		input::subscribe_asciiup([this](char ascii)
		{
			on_ascii_up(ascii);
		});
	}
}