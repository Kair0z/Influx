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
	class imgui_manager final
	{
	public:
		imgui_manager(graphics::device* device, texture* fonts_texture);

	private:
		void create_fonts_texture(graphics::device* device, texture* fonts_texture);
		void create_pipeline(graphics::device* device);

		graphics::pipeline* mp_pipeline;
		graphics::rootsignature* mp_rootsig;
		texture* mp_fonts_texture;
	};
}