#pragma once

namespace influx::renderer
{
	class texture;
	struct texture_desc;

	// private class used for resource management exclusively inside rendergraph
	class rgpool final
	{
		friend class rendergraph;
		rgpool() = default;

		void tick();
		texture* allocate_texture(const texture_desc& args);
		bool release_texture(texture* tex);
	};
}