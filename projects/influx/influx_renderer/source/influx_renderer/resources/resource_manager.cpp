#include "renderer_pch.h"
#include "resource_manager.h"

namespace influx::renderer
{
	resource_manager::resource_manager()
	{
        // dummy datas
		const string dummy_titles = "none";
        {
            texture_data dummy_data{};
            dummy_data.m_width = 256u;
            for (size_t i = 0u; i < 256u * 256u; ++i)
            {
                dummy_data.m_pixels.push_back(make_pixel32(255u, 255u, 255u, 255u));
            }
            load<e_resource_type::texture>(dummy_titles, dummy_data, false);
        }
		{
			cubemap_data dummy_data{};
			dummy_data.m_width = 256u;
			dummy_data.m_height = 256u;
			for (size_t i = 0u; i < 256u * 256u * 6u; ++i)
			{
				dummy_data.m_pixels.push_back(make_pixel32(255u, 255u, 255u, 255u));
			}
			load<e_resource_type::cubemap>(dummy_titles, dummy_data, false);
		}
	}

    void resource_manager::recreate_mesh(const string& title, detail::base_mesh_data* data)
    {
		graphics::device& device = renderer_backend::get_device();
		mesh_buffers*& meshbuffers = get_resource_map<e_resource_type::mesh>()[title].m_resource;
		if (meshbuffers == nullptr) meshbuffers = new mesh_buffers();

		// vertex buffer
		{
			const uint64 old_bytesize = meshbuffers->m_vertexbuffer ? meshbuffers->m_vertexbuffer->get_bytesize() : 0u;
			const uint64 new_bytesize = data->get_vert_bytesize();
			if (old_bytesize < new_bytesize)
			{
				// destroy old resource
				if (meshbuffers->m_vertexbuffer)
					device.release(meshbuffers->m_vertexbuffer);

				// create new vertex buffer on the shared heap
				graphics::heap_desc heap_desc{};
				heap_desc.m_type = graphics::e_heap_type::shared;
				graphics::buffer_desc desc{};
				desc.m_init_state = graphics::e_resource_state::gen_read;

				// create resource
				desc.m_bytesize = new_bytesize;
				desc.m_bytestride = data->get_vert_bytestride();
				meshbuffers->m_vertexbuffer = device.create_resource(desc, heap_desc);
				meshbuffers->m_vertexbuffer->set_name("vb_" + title);
			}

			// map new data to resource
			meshbuffers->m_vertexbuffer->map([data, new_bytesize](void* target)
			{
				memcpy(target, data->get_vert_data(), new_bytesize);
			});
		}
		// index buffer
		{
			const uint64 old_bytesize = meshbuffers->m_indexbuffer ? meshbuffers->m_indexbuffer->get_bytesize() : 0u;
			const uint64 new_bytesize = data->get_indx_bytesize();
			if (old_bytesize < new_bytesize)
			{
				// create index / vertex buffer on the shared heap (so cpu can write to it)
				graphics::heap_desc heap_desc{};
				heap_desc.m_type = graphics::e_heap_type::shared;
				graphics::buffer_desc desc{};
				desc.m_init_state = graphics::e_resource_state::gen_read;

				// create index buffer resource
				desc.m_bytesize = new_bytesize;
				desc.m_bytestride = data->get_indx_bytestride();
				desc.m_format = graphics::e_format::u32;
				meshbuffers->m_indexbuffer = device.create_resource(desc, heap_desc);
				meshbuffers->m_indexbuffer->set_name("ib_" + title);
			}

			meshbuffers->m_indexbuffer->map([data, new_bytesize](void* target)
			{
				memcpy(target, data->get_indx_data(), new_bytesize);
			});
		}
    }
    void resource_manager::recreate_texture(const string& title, const texture_data& data)
    {
		graphics::device& device = renderer_backend::get_device();
		upload_manager& uploadman = *renderer_backend::get_upload_manager();
		graphics::queue& queue = renderer_backend::get_graphics_queue();
		texture2D*& resource = get_resource_map<e_resource_type::texture>()[title].m_resource;

		if (resource != nullptr) delete resource;
		
		texture_desc create_args{};
		create_args.m_width = data.get_width();
		create_args.m_heigth = data.get_height();
		resource = new texture2D(&device, create_args);

		// make srv
		resource->m_srv = renderer_backend::get_descriptor_manager()->create_srv(resource->mp_resource);

		// upload to gpu
		uploadman.upload_texture(&queue, data, resource->get_resource());
    }
    void resource_manager::recreate_cubemap(const string& title, const cubemap_data& data)
    {
		graphics::device& device = renderer_backend::get_device();
		cubemap*& resource = get_resource_map<e_resource_type::cubemap>()[title].m_resource;
		graphics::queue& queue = renderer_backend::get_graphics_queue();

		if (resource != nullptr) delete resource;

		cubemap_desc create_args{};
		create_args.m_width = data.get_width();
		create_args.m_heigth = data.get_height();
		create_args.m_depth = data.get_depth();
		resource = new cubemap(&device, create_args);

		// make srv
		resource->m_srv = renderer_backend::get_descriptor_manager()->create_srv(resource->mp_resource);

		// upload to gpu
		graphics::commandlist& commandlist = *device.create_graphics_commandlist();
		commandlist.start(&device);
		resource->upload(commandlist, data);
		commandlist.end();
		commandlist.submit(&queue);
		commandlist.wait_for_completion();
    }

	resource_manager::~resource_manager()
	{
	}
}

