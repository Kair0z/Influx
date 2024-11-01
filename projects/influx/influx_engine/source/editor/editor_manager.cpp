#include "engine_pch.h"
#include "editor_manager.h"

namespace influx::engine
{
	editor_manager::editor_manager(editor_module* editor)
		: m_editor{ editor }
	{

	}

	void editor_manager::on_imgui(ImGuiContext& ctx)
	{
		m_editor->on_imgui(ctx);
	}
}