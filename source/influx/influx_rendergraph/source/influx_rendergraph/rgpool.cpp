#include "rendergraph_pch.h"
#include "rgpool.h"

namespace influx::rendergraph
{
#pragma region translation
	rhi_bufferdesc translate(const buffer_desc& desc)
	{
		rhi_bufferdesc new_desc{};
		new_desc.m_bytesize = desc.m_bytesize;
		new_desc.m_bytestride = desc.m_bytestride;
		new_desc.m_bindflags = desc.m_bindflags;
#if INFLUX_RG_BACKEND_GRAPHICS
		new_desc.m_format = desc.m_format;
#endif
		new_desc.m_init_state = desc.m_init_state;
		return new_desc;
	}
	rhi_texture2Ddesc translate(const texture_desc& desc)
	{
		rhi_texture2Ddesc new_desc{};
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
	rhi_descheap_type translate(const rgdescriptor_type& type)
	{
		switch (type)
		{
		case rgdescriptor_type::render_target: return rhi_descheap_type::rtv;
		case rgdescriptor_type::depth_target: return rhi_descheap_type::dsv;
		case rgdescriptor_type::read_only: return rhi_descheap_type::rsc;
		case rgdescriptor_type::read_write: return rhi_descheap_type::rsc;
		}
		return {};
	}
#pragma endregion

	rgpool::descriptor_list& rgpool::get_allocated_descriptors(e_descheap_slot slot)
	{
		const uint32 index = static_cast<uint32>(slot);
		return m_allocated_descriptors[index];
	}

	rgpool::rgpool(rhi_device& device, const global_config& config)
		: m_config{ config }
	{
		init_descriptor_heaps(device, config);
	}

	rgpool::~rgpool()
	{
		free_all_descriptors();
	}

	void rgpool::init_descriptor_heaps(rhi_device& device, const global_config& config)
	{
		auto create_and_store = [this, &device](e_descheap_slot slot, rhi_descheap::create_args args, uint32 capacity)
		{
#if INFLUX_RG_BACKEND_GRAPHICS
			args.m_capacity = capacity;
			get_descheap(slot) = device.create(args);
#endif
#if INFLUX_RG_BACKEND_RHI
			get_descheap(slot) = device.create(args).get();
#endif
		};

		// CPU heaps
		rhi_descheap::create_args args{};
		args.m_shader_visible = false;
		args.m_type = rhi_descheap_type::sampler;
		create_and_store(e_descheap_slot::samp, args, config.m_max_num_samplers);

		args.m_type = rhi_descheap_type::rsc;
		create_and_store(e_descheap_slot::rsc, args, config.m_max_num_srvs);

		args.m_type = rhi_descheap_type::rtv;
		create_and_store(e_descheap_slot::rtv, args, config.m_max_num_rtvs);

		args.m_type = rhi_descheap_type::dsv;
		create_and_store(e_descheap_slot::dsv, args, config.m_max_num_dsvs);

		// GPU heaps
		args.m_shader_visible = true;
		args.m_type = rhi_descheap_type::rsc;
		create_and_store(e_descheap_slot::rsc_gpu, args, config.m_max_num_srvs);

		args.m_type = rhi_descheap_type::sampler;
		create_and_store(e_descheap_slot::samp_gpu, args, config.m_max_num_samplers);
	}

	void rgpool::free_all_descriptors()
	{
		free_all_gpu_descriptors();

		// todo free cpu descriptors
	}

	void rgpool::free_all_gpu_descriptors()
	{
		get_descheap(e_descheap_slot::samp_gpu)->free_all();
		get_descheap(e_descheap_slot::rsc_gpu)->free_all();
	}

	void rgpool::cleanup(rhi_device& device)
	{
		free_all_descriptors();

		// destroy the heaps
		for (uint32 i = 0u; i < k_num_ext_descheap_slots; ++i)
			m_ext_descheaps[i] = nullptr;

		for (uint32 i = 0u; i < k_num_internal_descheaps; ++i)
		{
			m_int_descheaps[i]; // release this
		}
	}

	rhi_descheap*& rgpool::get_descheap(e_descheap_slot slot, bool ignore_external)
	{
		if (ignore_external)
		{
			const uint32 index = static_cast<uint32>(slot);
			return m_int_descheaps[index];
		}

		// do we have an ext heap override?
		rhi_descheap* ext_heap = nullptr;
		switch (slot)
		{
		case e_descheap_slot::rtv: ext_heap = m_ext_descheaps[static_cast<uint32>(e_ext_descheap_slot::rtv)]; break;
		case e_descheap_slot::dsv: ext_heap = m_ext_descheaps[static_cast<uint32>(e_ext_descheap_slot::dsv)]; break;
		case e_descheap_slot::rsc: ext_heap = m_ext_descheaps[static_cast<uint32>(e_ext_descheap_slot::resource)]; break;
		case e_descheap_slot::samp: ext_heap = m_ext_descheaps[static_cast<uint32>(e_ext_descheap_slot::sampler)]; break;
		}

		if (ext_heap)
		{
			return ext_heap;
		}
		else
		{
			const uint32 index = static_cast<uint32>(slot);
			return m_int_descheaps[index];
		}
	}

	result<rhi_resource*> rgpool::allocate_texture_resource(rhi_device& device, const texture_desc& args)
	{
		using result_type = result<rhi_resource*>;
		for (pooled_texture& item : m_texture_pool)
		{
			// if an item is active && matches most settings of args, just pass the existing resource
			if (!item.m_is_active && item.m_desc.is_recycle_match(args))
			{
				item.m_last_used_frame = m_frame;
				item.m_is_active = true;
				return item.m_resource;
			}
		}

		return result_type::make_error("failed allocating resource!");
#if 0
		// we failed recycling, time to allocate a new resource
		pooled_texture new_item{};
		new_item.m_is_active = true;
		new_item.m_last_used_frame = m_frame;
		new_item.m_resource = device.create( translate(args) );
		m_texture_pool.push_back(new_item);
		return new_item.m_resource;
#endif
	}

	result<rhi_resource*> rgpool::allocate_buffer_resource(rhi_device& device, const buffer_desc& args)
	{
		for (pooled_buffer& item : m_buffer_pool)
		{
			if (!item.m_is_active)
			{
				item.m_last_used_frame = m_frame;
				item.m_is_active = true;
				return item.m_resource;
			}
		}

		// create new
		pooled_buffer new_item{};
#if 0
		new_item.m_is_active = true;
		new_item.m_last_used_frame = m_frame;
		new_item.m_resource = device.create(translate(args));
		m_buffer_pool.push_back(new_item);
#endif
		return new_item.m_resource;
	}

	result<> rgpool::release_texture(rhi_device& device, rhi_resource& resource)
	{
		for (pooled_texture& item : m_texture_pool)
		{
			if (item.m_is_active && item.m_resource == &resource)
			{
				item.m_is_active = false;
				// device.release(&resource);
				return {};
			}
		}
		return result<>::make_error("error: release failed!");
	}

	result<> rgpool::release_buffer(rhi_device& device, rhi_resource& resource)
	{
		for (pooled_buffer& item : m_buffer_pool)
		{
			if (item.m_is_active && item.m_resource == &resource)
			{
				item.m_is_active = false;
				// device.release(&resource);
				return {};
			}
		}
		return result<>::make_error("error: release failed!");
	}

	result<rhi_descriptor> rgpool::alloc_cpu_descriptor(rgdescriptor_type type)
	{
		using result_type = result<rhi_descriptor>;

		rhi_descheap* heap = nullptr;
		switch (type)
		{
		case rgdescriptor_type::render_target: heap = get_descheap(e_descheap_slot::rtv); break;
		case rgdescriptor_type::depth_target: heap = get_descheap(e_descheap_slot::dsv); break;
		case rgdescriptor_type::read_only: heap = get_descheap(e_descheap_slot::rsc); break;
		case rgdescriptor_type::read_write: heap = get_descheap(e_descheap_slot::rsc); break;
		}

		if (heap == nullptr)
			return result_type::make_error("invalid descriptor type!");

#if INFLUX_RG_BACKEND_RHI
		auto alloc = heap->allocate(1u);
		if (!alloc) return result_type::make_error("failed allocating a descriptor!");
		return heap->get_cpu_descriptor(alloc.get());
#else
		return result_type::make_error("not implemented!");
#endif
	}

	result<rhi_descriptor> rgpool::alloc_gpu_srv()
	{
		using result_type = result<rhi_descriptor>;
#if INFLUX_RG_BACKEND_RHI
		rhi_descheap& heap = get_descheap(e_descheap_slot::rsc_gpu);
		uint32 alloc_index = heap.allocate(1u).get();
		return heap.get_gpu_descriptor(alloc_index);
#else
		return result_type::make_error("no impl!");
#endif
	}

	result<rhi_descriptor> rgpool::alloc_gpu_sampler()
	{
		using result_type = result<rhi_descriptor>;
#if INFLUX_RG_BACKEND_RHI
		rhi_descheap& heap = get_descheap(e_descheap_slot::samp_gpu);
		uint32 alloc_index = heap.allocate(1u).get();
		return heap.get_gpu_descriptor(alloc_index);
#endif
		return result_type::make_error("no impl!");
	}

	result<> rgpool::free_descriptor(e_descheap_slot type, rhi_descriptor handle)
	{
		list<rhi_descriptor>& descriptors = get_allocated_descriptors(type);
		if (descriptors.find(handle))
		{
			get_descheap(type)->free(handle);
			descriptors.remove(handle);
		}
		
		return result<>::make_error("in.handle was not allocated by us!");
	}

	result<> rgpool::bind_ext_descheap(e_ext_descheap_slot slot, rhi_descheap& heap, bool allow_override)
	{
		using result_type = result<>;

		const uint32 index = static_cast<uint32>(heap.get_type());
		if (m_ext_descheaps[index] != nullptr)
		{
			if (allow_override) unbind_ext_descheap(slot);
			else return result_type::make_error("descheap at slot is already bound!");
		}
		
		m_ext_descheaps[index] = &heap;
		return {};
	}
	result<> rgpool::unbind_ext_descheap(e_ext_descheap_slot slot)
	{
		const uint32 index = static_cast<uint32>(slot);
		m_ext_descheaps[index] = nullptr;
		return {};
	}
	bool rgpool::is_ext_descheap_bound(e_ext_descheap_slot slot) const
	{
		const uint32 index = static_cast<uint32>(slot);
		return m_ext_descheaps[index] != nullptr;
	}
}