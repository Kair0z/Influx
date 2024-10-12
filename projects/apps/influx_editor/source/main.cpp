#include "influx_engine.h"
#include "engine/entrypoint.h"

class editor final : public influx::engine::editor_module
{
public:
	virtual void on_imgui() override
	{

	}
};
influx_engine_editor(editor);