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

		graphics::buffer_desc texture_desc{};
		texture_desc.m_bytesize = 1024u * 1024u * 14u;
		texture_desc.m_init_state = graphics::e_resource_state::read;
		mp_texture_upload_resource = mp_device->create_resource(texture_desc, heap_desc);

		graphics::buffer_desc buffer_desc{};
		buffer_desc.m_bytesize = 1024u * 1024u * 14u;
		buffer_desc.m_init_state = graphics::e_resource_state::read;
		mp_buffer_upload_resource = mp_device->create_resource(buffer_desc, heap_desc);

		mp_fence = mp_device->create_fence(0u);
		mp_commandlist = mp_device->create_graphics_commandlist();
	}

	void upload_manager::upload_buffer(graphics::queue* queue, const vector<byte>& data, graphics::resource* target)
	{
		map_buffer(data);

		const uint32 num_bytes = (uint32)data.size();

		// start a commandlist that copies the buffer from intermediate -> gpu resource
		mp_commandlist->start(mp_device, nullptr);
		{
			// transition our gpu buffer to copy_dest
			target->transition(mp_commandlist, graphics::e_resource_state::copy_dest);

			graphics::copy_buffer_args copy_args{};
			mp_commandlist->copy_buffer(
				mp_buffer_upload_resource, target, num_bytes, copy_args);

			// transition our gpu texture to shader resource usage
			target->revert_transition(mp_commandlist);
		}
		mp_commandlist->end();

		queue->submit_commandlists({ mp_commandlist });
		queue->queue_signal(mp_fence, 1u);

		// wait for the signal
		wait_handle handle{};
		mp_fence->wait_for_value(1u, handle);

		queue->queue_signal(mp_fence, 0u);
	}

	void upload_manager::upload_texture(graphics::queue* queue, const texture_data& data, graphics::resource* target_resource)
	{
		const size_t texture_bytesize = data.m_pixels.size() * sizeof(pixel32);
		static uint32 num_textures = 0u;

		const range<size_t> upload_subrange{
			(num_textures++ * texture_bytesize), texture_bytesize };

		// MAP texture data onto the upload resource
		graphics::map_args args{};
		args.m_begin = upload_subrange.get_start();
		args.m_end = upload_subrange.get_end();
		mp_texture_upload_resource->map([&data, texture_bytesize](void* target)
		{
			memcpy(target, data.m_pixels.data(), texture_bytesize);

		}, args);

		// start a commandlist that copies the texture from intermediate -> gpu resource
		mp_commandlist->start(mp_device, nullptr);
		{
			// transition our gpu texture to shader resource usage
			mp_commandlist->transition_resource(target_resource,
				graphics::e_resource_state::shader_resource,
				graphics::e_resource_state::copy_dest);

			graphics::copy_texture_args copy_args{};
			copy_args.m_src.m_range = upload_subrange;
			copy_args.m_dest.m_range = target_resource->get_full_range();

			mp_commandlist->copy_texture(
				mp_texture_upload_resource, target_resource, copy_args);

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
		delete mp_texture_upload_resource;
		delete mp_buffer_upload_resource;
		delete mp_fence;
		delete mp_commandlist;
	}

	void upload_manager::map_buffer(const vector<byte>& data)
	{
		const size_t num_bytes = data.size();

		graphics::map_args args{};
		args.m_end = num_bytes;
		mp_buffer_upload_resource->map([&data, num_bytes](void* target)
		{
			memcpy(target, data.data(), num_bytes);
		}, 
		args);
	}
}