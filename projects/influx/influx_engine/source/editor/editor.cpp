#include "engine_pch.h"
#include "imgui/imgui.h"

#include "file/engine_files.h"

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

	void open_gamefile(const string& game_name)
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

	void editor_module::on_config(app_config&, editor_config&)
	{
	}

	void editor_module::on_imgui(ImGuiContext& ctx)
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("file"))
			{
				if (ImGui::Button("new"))
				{
					create_gamefile("influx_game");
				}

				if (ImGui::Button("open"))
				{
					open_gamefile("influx_game");
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("edit"))
			{
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void editor_module::on_cleanup()
	{
	}
}