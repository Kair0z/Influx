#pragma once

namespace influx::graphics
{
	class device;
	class descriptor_heap;
	class rootsignature;
	class pipeline;
}

namespace influx::renderer
{
	class texture;

	class imgui_manager final
	{
	public:
		imgui_manager(graphics::device* device);

	private:
		void create_fonts_texture(graphics::device* device);
		void create_pipeline(graphics::device* device);

		graphics::pipeline* mp_pipeline;
		graphics::rootsignature* mp_rootsig;
		texture* mp_fonts_texture;
	};
}