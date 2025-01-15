#pragma once

// influx::core
#include "core/container/vector.h"

// influx::graphics
namespace influx::graphics
{
	class commandlist;
}

// influx::renderer
namespace influx::renderer
{
	struct scene_shadertoy;
	class target;
}

namespace influx::renderer
{
	class shadertoy_renderer final
	{
	public:
		shadertoy_renderer();
		~shadertoy_renderer();

		void render(
			graphics::commandlist* commandlist,
			const scene_shadertoy& scene,
			const target& target);

	private:
		graphics::resource* mp_vertexbuffer;
		graphics::resource* mp_indexbuffer;
	};
}