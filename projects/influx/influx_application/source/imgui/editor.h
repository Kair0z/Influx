#pragma once

struct ImDrawData;

namespace influx::application
{
	class editor final
	{
	public:
		editor();

		void set_window_dimensions(const math::vectorf2& dimensions);

		void update();

		ImDrawData* get_imgui_drawdata(float target_width, float target_height);
	};
}