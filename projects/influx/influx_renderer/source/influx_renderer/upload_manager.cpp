#include "renderer_pch.h"
#include "upload_manager.h"
#include "influx_graphics/device.h"

namespace influx::renderer
{
	upload_manager::upload_manager(graphics::device* device)
		: mp_device{ device }
	{
		graphics::heap_desc heap_desc{};
		heap_desc.m_type = graphics::e_heap_type::shared;

		graphics::buffer_desc desc{};
		desc.m_bytesize = 16u * 1024u * 1024u;
		desc.m_init_state = graphics::e_resource_state::read;

		mp_upload_resource = mp_device->create_resource(desc, heap_desc);

		mp_fence = mp_device->create_fence(0u);
		mp_commandalloc = mp_device->create_graphics_allocator();
		mp_commandlist = mp_device->create_graphics_command_list(mp_commandalloc);
	}

	void upload_manager::upload_texture(graphics::command_queue* queue, const texture_data& data, graphics::resource* target_resource)
	{
		static uint32 num_textures = 0u;
		const size_t texture_bytesize = data.m_pixels.size() * sizeof(byte);

		// MAP texture data onto the upload resource
		graphics::map_args args{};
		args.m_begin = texture_bytesize * num_textures++;
		args.m_end = args.m_begin + texture_bytesize;
		mp_upload_resource->map([&data, texture_bytesize](void* target)
		{
			memcpy(target,
				data.m_pixels.data(),
				texture_bytesize);
		}, args);

		// start a commandlist that copies the texture from intermediate -> gpu resource
		mp_commandlist->start(mp_commandalloc, nullptr);
		{
			// transition our gpu texture to shader resource usage
			mp_commandlist->transition_resource(target_resource,
				graphics::e_resource_state::shader_resource,
				graphics::e_resource_state::copy_dest);

			mp_commandlist->copy_texture(
				mp_upload_resource, target_resource);

			// transition our gpu texture to shader resource usage
			mp_commandlist->transition_resource(target_resource, 
				graphics::e_resource_state::copy_dest,
				graphics::e_resource_state::shader_resource);
		}
		mp_commandlist->end();

		queue->submit_commandlists({ mp_commandlist });
		queue->queue_signal(mp_fence, 1u);
		
		// wait for the signal
		wait_handle handle{};
		mp_fence->wait_for_value(1u, handle);

		queue->queue_signal(mp_fence, 0u);
	}

	upload_manager::~upload_manager()
	{
		delete mp_upload_resource;
		delete mp_fence;
		delete mp_commandalloc;
		delete mp_commandlist;
	}
}