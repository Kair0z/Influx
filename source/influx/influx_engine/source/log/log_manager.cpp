#include "engine_pch.h"
#include "log_manager.h"

// influx::engine
#include "file/engine_files.h"
#include "influx_imgui/imgui_widgets.h"
#include "editor/editor_manager.h"

// influx::core
#include "core/log.h"
#include "core/math/math.h"

namespace influx::engine
{
	static string g_filepath = "";

	class log_editor final : public editor::editor_window
	{
	public:
		virtual void on_prerun() override
		{
			ImGui::SetNextWindowBgAlpha(0.0);
		}

		virtual void on_run() override
		{
			set_name("log");
			m_logger.draw();

			for (const string& line : path::get_lines(g_filepath, m_num_lines_read).get())
			{
				m_logger.push((line + "\n").c_str(), {});
				++m_num_lines_read;
			}
		}

	private:
		imgui::logger m_logger;
		uint32 m_num_lines_read = 0u;
	};

	log_manager::log_manager()
	{
		static string intermediate = to_string(get_engine_directory(engine_directory::intermediate).get_full_path());
		g_filepath = intermediate + "/log/engine.log";

		m_categories.resize(k_capacity);
		m_lines.resize(k_capacity);

		if (path::exists(g_filepath))
		{
			path::duplicate_file(g_filepath);
			path::clear_content(g_filepath);
		}
		else
		{
			path::create_file(g_filepath);
		}

		m_linecount = 0;

		editor::editor_manager::static_window<log_editor>("log");
	}

	log_manager::~log_manager()
	{
		flush_to_file();
	}

	void log_manager::tick()
	{
		flush_to_file();
	}

	void log_manager::flush_to_file(uint32 max_num_lines)
	{
		uint32 num_flushed = 0u;
		max_num_lines = math::minimum(m_linecount, max_num_lines);

		path::scoped_push_lines(g_filepath, [this, &num_flushed, max_num_lines]
		(std::ofstream& file)
		{
			for (num_flushed; num_flushed < max_num_lines; ++num_flushed)
			{
				file << m_lines[num_flushed] << "\n";
			}
		});

		m_linecount -= num_flushed;
	}
}