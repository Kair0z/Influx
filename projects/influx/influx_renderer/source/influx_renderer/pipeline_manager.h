#pragma once

namespace influx::graphics
{
	class device;
}

namespace influx::renderer
{
	class pipeline;
}

namespace influx::renderer
{
	class pipeline_manager final
	{
	public:
		pipeline_manager(graphics::device* device);

	private:
		vector<pipeline*> mp_pipelines{};
	};
}