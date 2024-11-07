#include "engine_pch.h"
#include "editor_manager.h"

// influx::core
#include "core/log.h"

// influx::engine
#include "file/engine_files.h"

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

	// creates file at /influx/games/'game_name'/
	void create_gamefile(const string& game_name)
	{
		const string gamefile_path = make_gamefile_path(game_name);
		if (file::exists(gamefile_path))
		{
			// already exists
		}
		else
		{
			// new file
			file_game gamefile{};
			gamefile.m_name = game_name;
			gamefile.m_id = 0u;
			gamefile.save(gamefile_path);
		}
	}

	// loads file at /influx/games/'game_name'/
	void load_gamefile(const string& game_name)
	{
		const string gamefile_path = make_gamefile_path(game_name);
		if (file::exists(gamefile_path))
		{
			// open existing file

		}
		else
		{

		}
	}

	editor_manager::editor_manager(editor_module* editor)
		: m_editor{ editor }
	{
		m_editor_toggle.force_set(true);

		initialize_inputs();
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

	void editor_manager::on_imgui(ImGuiContext& ctx)
	{
		process_inputs();

		if (!m_editor_toggle)
		{
			return;
		}

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("file"))
			{
				if (ImGui::Button("new"))
				{
					create_gamefile("influx_game");
				}

				if (ImGui::Button("load"))
				{
					load_gamefile("influx_game");
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
			}
			ImGui::End();
		}
		
		if (m_content_toggle)
		{
			if (ImGui::Begin("content"))
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