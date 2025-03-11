#pragma once

// influx::renderer
namespace influx::renderer
{
	class renderer_backend;
	class target;
}

// influx::graphics
namespace influx::graphics
{
	class device;
	class commandlist;
	class descriptor_heap;
	class resource;
	class shader_resource_view;
}

namespace influx::renderer
{
	class quad_renderer final
	{
	public:
		quad_renderer();

		~quad_renderer();

		void render_quad(graphics::commandlist* commandlist, const target& target);

	private:
		graphics::resource* mp_vertexbuffer;
		graphics::resource* mp_indexbuffer;
	};
}