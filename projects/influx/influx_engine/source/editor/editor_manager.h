#pragma once

namespace influx::engine
{
	class editor_manager final
	{
	public:
		editor_manager(editor_module* editor);

		void on_imgui(ImGuiContext& ctx);

	private:
		editor_module* m_editor = nullptr;
	};
}