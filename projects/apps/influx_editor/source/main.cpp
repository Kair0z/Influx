#include "influx_engine.h"
#include "imgui/imgui.h"

class editor final : public influx::engine::editor_module
{
public:
	virtual void on_config(influx::engine::app_config& app, influx::engine::editor_config& ed) override
	{
		app.set_window_dim({ 640u, 480u });
	}

	virtual void on_imgui(ImGuiContext& ctx) override
	{
		editor_module::on_imgui(ctx);

#if 0
		ImGui::SetCurrentContext(&ctx);

		if (ImGui::Begin("custom editor"))
		{
			
		}
		ImGui::End();
#endif
	}
};
influx_engine_editor(editor);
