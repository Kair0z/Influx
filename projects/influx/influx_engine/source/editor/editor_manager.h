#pragma once

namespace influx::engine
{
	class editor_manager final
	{
	public:
		editor_manager();

		void on_imgui(ImGuiContext& ctx);
	};
}