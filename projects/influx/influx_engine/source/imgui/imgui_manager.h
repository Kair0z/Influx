#pragma once

namespace influx::engine
{
	class imgui_manager final
	{
	public:
		imgui_manager();

		void on_window_resize(const math::vectoru2& new_size);
		void new_frame();
		void render();
		void present();

		~imgui_manager();
	};
}