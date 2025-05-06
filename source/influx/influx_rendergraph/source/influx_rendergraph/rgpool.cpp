#include "rendergraph_pch.h"
#include "rgpool.h"

// influx::graphics
#include "influx_graphics/device.h"

namespace influx::rendergraph
{
#pragma region translation
	graphics::buffer_desc translate(const buffer_desc& desc)
	{
		graphics::buffer_desc new_desc{};
		new_desc.m_bytesize = desc.m_bytesize;
		new_desc.m_bytestride = desc.m_bytestride;
		new_desc.m_bindflags = desc.m_bindflags;
		new_desc.m_format = desc.m_format;
		new_desc.m_init_state = desc.m_init_state;
		return new_desc;
	}
	graphics::tex2D_desc translate(const texture_desc& desc)
	{
		graphics::tex2D_desc new_desc{};
		new_desc.m_arraysize = desc.m_array_size;
		new_desc.m_dimensions = { desc.m_width, desc.m_heigth };
		new_desc.m_bindflags = desc.m_bindflags;
		new_desc.m_format = desc.m_format;
		new_desc.m_init_state = desc.m_init_state;
		new_desc.m_num_mips = desc.m_num_mips;
		new_desc.m_sample_count = desc.m_sample_count;
		new_desc.m_allow_uav = desc.m_allow_uav;
		return new_desc;
	}
#pragma endregion

	rgpool::rgpool(graphics::device& device, const global_config& config)
		: m_config{ config }
	{
		create_descriptor_heaps(device, config);
	}

	rgpool::~rgpool()
	{
		free_all_descriptors();
	}

	void rgpool::create_descriptor_heaps(graphics::device& device, const global_config& config)
	{
		graphics::descriptor_heap::create_args args{};
		args.m_shader_visible = false;

		args.m_capacity = config.m_max_num_samplers;
		args.m_type = graphics::e_descriptor_heap_type::sampler;
		m_sampler_heap = device.create_descriptor_heap(args);

		args.m_capacity = config.m_max_num_srvs;
		args.m_type = graphics::e_descriptor_heap_type::srv;
		m_srv_heap = device.create_descriptor_heap(args);

		// non-shader heaps:
		args.m_shader_visible = false;

		args.m_capacity = config.m_max_num_rtvs;
		args.m_type = graphics::e_descriptor_heap_type::rtv;
		m_rtv_heap = device.create_descriptor_heap(args);

		args.m_capacity = config.m_max_num_dsvs;
		args.m_type = graphics::e_descriptor_heap_type::dsv;
		m_dsv_heap = device.create_descriptor_heap(args);
	}

	void rgpool::free_all_descriptors()
	{
		free_all_gpu_descriptors();
		m_rtv_heap->free_all_cpu();
		m_dsv_heap->free_all_cpu();
	}

	void rgpool::free_all_gpu_descriptors()
	{
		m_srv_heap->free_all_gpu();
		m_srv_heap->free_all_cpu();
		m_sampler_heap->free_all_cpu();
		m_sampler_heap->free_all_gpu();
	}

	void rgpool::cleanup(graphics::device& device)
	{
		device.release(m_srv_heap);
		device.release(m_sampler_heap);
		device.release(m_rtv_heap);
		device.release(m_dsv_heap);
	}

	void rgpool::recycle_resources()
	{
		const uint32 frames_until_recycle = m_config.m_frames_until_resource_recycle;
		for (uint64 i = 0; i < m_texture_pool.size();)
		{
			pooled_resource& resource = m_texture_pool[i];
			const bool is_expired = m_frame > resource.m_last_used_frame + frames_until_recycle;

			if (!resource.m_is_active && is_expired)
			{
				// remove element from the texture pool
				std::swap(m_texture_pool[i], m_texture_pool.back());
				m_texture_pool.pop_back();
				break;
			}
			else ++i;
		}

		++m_frame;
	}

	result<graphics::resource*> rgpool::allocate_texture_resource(graphics::device& device, const texture_desc& args)
	{
		for (pooled_resource& item : m_texture_pool)
		{
			if (!item.m_is_active)
			{
				item.m_last_used_frame = m_frame;
				item.m_is_active = true;
				return item.m_resource;
			}
		}

		// create new
		pooled_resource new_item{};
		new_item.m_is_active = true;
		new_item.m_last_used_frame = m_frame;
		new_item.m_resource = device.create_resource(translate(args));
		m_texture_pool.push_back(new_item);
		return new_item.m_resource;
	}

	result<graphics::resource*> rgpool::allocate_buffer_resource(graphics::device& device, const buffer_desc& args)
	{
		for (pooled_resource& item : m_buffer_pool)
		{
			if (!item.m_is_active)
			{
				item.m_last_used_frame = m_frame;
				item.m_is_active = true;
				return item.m_resource;
			}
		}

		// create new
		pooled_resource new_item{};
		new_item.m_is_active = true;
		new_item.m_last_used_frame = m_frame;
		new_item.m_resource = device.create_resource(translate(args));
		m_buffer_pool.push_back(new_item);
		return new_item.m_resource;
	}

	result<> rgpool::release_texture(graphics::device& device, graphics::resource& resource)
	{
		for (pooled_resource& item : m_texture_pool)
		{
			if (item.m_is_active && item.m_resource == &resource)
			{
				item.m_is_active = false;
				device.release(&resource);
				return {};
			}
		}
		return result<>::make_error("error: release failed!");
	}

	result<> rgpool::release_buffer(graphics::device& device, graphics::resource& resource)
	{
		for (pooled_resource& item : m_buffer_pool)
		{
			if (item.m_is_active && item.m_resource == &resource)
			{
				item.m_is_active = false;
				device.release(&resource);
				return {};
			}
		}
		return result<>::make_error("error: release failed!");
	}

	result<graphics::descriptor_handle> rgpool::alloc_cpu_handle(rgdescriptor_type type)
	{
		switch (type)
		{
		case rgdescriptor_type::render_target: return m_rtv_heap->allocate_cpu();
		case rgdescriptor_type::depth_target: return m_dsv_heap->allocate_cpu();
		case rgdescriptor_type::read_only: return m_srv_heap->allocate_cpu();
		case rgdescriptor_type::read_write: return m_srv_heap->allocate_cpu();
		}

		return {};
	}

	result<graphics::descriptor_handle> rgpool::alloc_gpu_srv()
	{
		return m_srv_heap->allocate_gpu();
	}

	result<graphics::descriptor_handle> rgpool::alloc_gpu_sampler()
	{
		return m_sampler_heap->allocate_gpu();
	}
}