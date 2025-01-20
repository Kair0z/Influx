#pragma once

#include "core/basetypes.h"
#include "core/container/vector.h"

namespace influx::graphics
{
	class device;
	class resource;
}

namespace influx::rendergraph
{
	struct texture_desc;
	struct buffer_desc;

	// private class used for resource management exclusively inside rendergraph
	class rgpool final
	{
		struct pooled_resource final
		{
			graphics::resource* m_resource;
			uint64 m_last_used_frame;
			bool m_is_active;
		};

		friend class rendergraph;
		rgpool(graphics::device* device);

		void tick();

		graphics::resource* allocate_texture_resource(const texture_desc& args);
		graphics::resource* allocate_buffer_resource(const buffer_desc& args);
		bool release_texture(graphics::resource* resource);
		bool release_buffer(graphics::resource* resource);

		graphics::device* m_device;
		uint64 m_frame = 0u;
		vector<pooled_resource> m_texture_pool;
		vector<pooled_resource> m_buffer_pool;
	};
}