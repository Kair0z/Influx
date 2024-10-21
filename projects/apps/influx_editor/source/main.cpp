#include "influx_engine.h"
#include "engine/entrypoint.h"
#include "core/scope.h"

#include "imgui/imgui.h"

class editor final : public influx::engine::editor_module
{
public:
	virtual void on_config(influx::engine::app_config& app, influx::engine::editor_config& ed) override
	{
		app
			.set_window_dim({ 640u, 480u });
	}

	virtual void on_imgui() override
	{
		editor_module::on_imgui();

		if (ImGui::Begin("influx editor"))
		{
			// ...
			ImGui::End();
		}
	}
};
influx_engine_editor(editor);
