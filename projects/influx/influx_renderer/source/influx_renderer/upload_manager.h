#pragma once

namespace influx::graphics
{
	class device;
	class resource;
	class commandlist;
	class queue;
	class fence;
}

namespace influx::renderer
{
	class upload_manager final
	{
	public:
		upload_manager(graphics::device* device);

		void upload_buffer(graphics::queue* queue, const vector<byte>& data, graphics::resource* target);
		void upload_texture(graphics::queue* queue, const texture_data& data, graphics::resource* target_resource);
		void upload_texture(graphics::queue* queue, const texturecube_data& data, graphics::resource* target_resource);

		virtual ~upload_manager();

	private:
		graphics::device* mp_device;
		graphics::commandlist* mp_commandlist;
		graphics::fence* mp_fence;

		graphics::resource* mp_texture_upload_resource;
		graphics::resource* mp_buffer_upload_resource;

	private:
		void map_buffer(const vector<byte>& data);
		void map_texture(const texture_data& data);
	};
}