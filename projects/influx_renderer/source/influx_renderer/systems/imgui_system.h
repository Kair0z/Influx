#pragma once
#include "influx_renderer/rendersystem.h"

namespace influx::graphics
{
	class device;
}

namespace influx::renderer
{
	class imgui_system final : public rendersystem
	{
	public:
		imgui_system(graphics::device* device);
		virtual ~imgui_system();

		virtual void initialize() override;

		virtual void tick() override;

	private:

	};
}