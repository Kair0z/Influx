#pragma once

struct ImGuiContext;

namespace influx::engine
{
	class INFLUX_ENGINE_API editor_module : public base_module
	{
	public:
		virtual void on_config(app_config&, editor_config&);
		virtual void on_imgui(ImGuiContext& ctx);
		virtual void on_cleanup();

		virtual ~editor_module() = default;
	};
}