#include "renderer_pch.h"
#include "resource_manager.h"

// influx::renderer
#include "influx_renderer/mesh.h"

// embedded shaders
namespace emb {
#include "../shaders/embedded_shaders.h"
}

namespace influx::renderer
{
	resource_manager::resource_manager()
	{
		load_internal_resources();
	}

	void resource_manager::load_internal_resources()
	{
		// textures
		const cubemap_id tex_none = get_internal_texture_id(e_texture::none);
		{
			const tex_id tex_none = get_internal_texture_id(e_texture::none);
			texture2D_data dummy_data{};
			dummy_data.m_width = 256u;
			for (size_t i = 0u; i < 256u * 256u; ++i)
			{
				dummy_data.m_pixels.push_back(make_pixel32(255u, 255u, 255u, 255u));
			}
			load<e_resource_type::texture2D>(tex_none, dummy_data, false);
		}
		{
			const tex_id tex_none = get_internal_texture_id(e_texture::none);
			texture3D_data dummy_data{};
			static const uint32 k_hard_dimensions = 256u;
			dummy_data.m_width = dummy_data.m_height = k_hard_dimensions;
			for (size_t i = 0u; i < k_hard_dimensions * k_hard_dimensions * k_hard_dimensions; ++i)
			{
				dummy_data.m_pixels.push_back(make_pixel32(255u, 255u, 255u, 255u));
			}
			load<e_resource_type::texture3D>(tex_none, dummy_data, false);
		}
		{
			cubemap_data dummy_data{};
			dummy_data.m_width = 256u;
			dummy_data.m_height = 256u;
			for (size_t i = 0u; i < 256u * 256u * 6u; ++i)
			{
				dummy_data.m_pixels.push_back(make_pixel32(255u, 255u, 255u, 255u));
			}
			load<e_resource_type::cubemap>(tex_none, dummy_data, false);
		}
		
		// shaders
		{
			static const auto load_shader = [this](
				const string& shadername, const string& entrypoint,
				unsigned char* bytecode, uint64 bytecode_length,
				unsigned char* refl_blob, uint64 refl_blob_length)
			{
				shader::compile_output compile_output{};
				compile_output.m_bytecode.resize(bytecode_length);
				memcpy(compile_output.m_bytecode.data(), bytecode, bytecode_length);

				shader::reflection& reflection = compile_output.m_reflection;
				shader::reflection::deserialize(reflection, refl_blob, refl_blob_length);

				shader::shader_signature& signature = compile_output.m_signature;
				signature.set_entrypoint(entrypoint);
				signature.set_filename(shadername);
				signature.set_type(reflection.m_shader_type);
				signature.set_target(k_shader_target);
				const shader_id id = make_shader_id(signature);
				load<e_resource_type::shader>(id, shader_data::translate(compile_output), true);
			};

			load_shader("basepass", "main_vs", emb::basepass_main_vs_cso, emb::basepass_main_vs_cso_len, emb::basepass_main_vs_refl, emb::basepass_main_vs_refl_len);
			load_shader("basepass", "main_ps", emb::basepass_main_ps_cso, emb::basepass_main_ps_cso_len, emb::basepass_main_ps_refl, emb::basepass_main_ps_refl_len);
			load_shader("resolvepass", "main_cs", emb::resolvepass_main_cs_cso, emb::resolvepass_main_cs_cso_len, emb::resolvepass_main_cs_refl, emb::resolvepass_main_cs_refl_len);
		}

		// meshes
		load<e_resource_type::mesh>( get_internal_mesh_id(e_mesh::placeholder)	, &get_inline_mesh<e_mesh::placeholder>(), true);
		load<e_resource_type::mesh>( get_internal_mesh_id(e_mesh::box)			, &get_inline_mesh<e_mesh::box>(), true);
		load<e_resource_type::mesh>( get_internal_mesh_id(e_mesh::plane)		, &get_inline_mesh<e_mesh::plane>(), true);
		load<e_resource_type::mesh>( get_internal_mesh_id(e_mesh::quad)			, &get_inline_mesh<e_mesh::quad>(), true);
		load<e_resource_type::mesh>( get_internal_mesh_id(e_mesh::sphere)		, &get_inline_mesh<e_mesh::sphere>(), true);
		load<e_resource_type::mesh>( get_internal_mesh_id(e_mesh::triangle)		, &get_inline_mesh<e_mesh::triangle>(), true);
	}

    void resource_manager::recreate_mesh(const mesh_id& id, detail::base_mesh_data const* data)
    {
		influx_assert(data != nullptr);

		mesh_buffers*& meshbuffers = get_resource_map<e_resource_type::mesh>()[id].m_resource;
		if (meshbuffers == nullptr) meshbuffers = new mesh_buffers();
		debug_name& name = m_mesh_map[id].m_debugname;
		
		if (is_internal_mesh(id))
			name = get_internal_mesh_name(get_internal_mesh(id));

		graphics::device& device = renderer_backend::get_device();
		
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

				const bool internal_mesh = is_internal_mesh(id);
				const string mesh_name;
				meshbuffers->m_vertexbuffer->set_name("vb_" + name.get_string());
			}

			// map new data to resource
			if (new_bytesize > 0u)
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

				meshbuffers->m_indexbuffer->set_name("ib_" + name.get_string());
			}

			if (new_bytesize > 0u)
			meshbuffers->m_indexbuffer->map([data, new_bytesize](void* target)
			{
				memcpy(target, data->get_indx_data(), new_bytesize);
			});
		}
    }
    void resource_manager::recreate_texture2D(const tex_id& id, const texture2D_data& data)
    {
		graphics::device& device = renderer_backend::get_device();
		upload_manager& uploadman = *renderer_backend::get_upload_manager();
		graphics::queue& queue = renderer_backend::get_graphics_queue();
		texture2D*& resource = get_resource_map<e_resource_type::texture2D>()[id].m_resource;

		if (resource != nullptr)
			delete resource;
		
		texture_desc create_desc{};
		create_desc.m_width = data.get_width();
		create_desc.m_heigth = data.get_height();
		resource = new texture2D(device, create_desc);

		// make srv
		resource->m_srv = renderer_backend::get_descriptor_manager()->create_srv(device, *resource->m_resource);

		// upload to gpu
		uploadman.upload_texture(&queue, data, resource->get_resource().get());
    }
	void resource_manager::recreate_texture3D(const tex_id& id, const texture3D_data& data)
	{
	}
    void resource_manager::recreate_cubemap(const cubemap_id& id, const cubemap_data& data)
    {
		graphics::device& device = renderer_backend::get_device();
		cubemap*& resource = get_resource_map<e_resource_type::cubemap>()[id].m_resource;
		graphics::queue& queue = renderer_backend::get_graphics_queue();

		if (resource != nullptr) 
			delete resource;

		cubemap_desc create_desc{};
		create_desc.m_width = data.get_width();
		create_desc.m_heigth = data.get_height();
		resource = new cubemap(device, create_desc);

		// make srv
		resource->m_srv = renderer_backend::get_descriptor_manager()->create_srv(device, *resource->m_resource);

		// upload to gpu
		graphics::commandlist& commandlist = *device.create_graphics_commandlist();
		commandlist.start_recording(&device);
		resource->upload(device, commandlist, data);
		commandlist.end_recording();
		commandlist.submit(&queue);
		commandlist.wait_for_completion();
    }
	void resource_manager::recreate_shader(const shader_id& sig, const shader_data& data)
	{

	}
	resource_manager::~resource_manager()
	{
	}
}

