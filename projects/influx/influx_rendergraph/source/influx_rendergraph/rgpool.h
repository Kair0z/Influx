#pragma once

namespace influx::graphics
{
	class device;
}

namespace influx::rendergraph
{
	class texture;
	struct texture_desc;

	// private class used for resource management exclusively inside rendergraph
	class rgpool final
	{
		friend class rendergraph;
		rgpool(graphics::device* device);

		void tick();
		texture* allocate_texture(const texture_desc& args);
		bool release_texture(texture* tex);
	};
}