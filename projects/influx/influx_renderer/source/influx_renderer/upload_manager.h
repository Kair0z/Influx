#pragma once

namespace influx::graphics
{
	class device;
	class resource;
	class command_list;
	class command_allocator;
	class command_queue;
	class fence;
}

namespace influx::renderer
{
	class upload_manager final
	{
	public:
		upload_manager(graphics::device* device);

		void upload_texture(graphics::command_queue* queue, const texture_data& data, graphics::resource* target_resource);

		virtual ~upload_manager();

	private:
		graphics::device* mp_device;
		graphics::command_list* mp_commandlist;
		graphics::command_allocator* mp_commandalloc;
		graphics::fence* mp_fence;

		graphics::resource* mp_upload_resource;
	};
}