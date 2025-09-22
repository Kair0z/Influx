#include "renderer_pch.h"
#include "descriptor_manager.h"

// influx::renderer
#include "influx_renderer/texture.h"

// influx::graphics
#include "influx_graphics/device.h"
#include "influx_graphics/descriptors.h"

namespace influx::renderer
{
	// https://learn.microsoft.com/en-us/windows/win32/direct3d12/hardware-support
	constexpr static uint64 k_cpu_heap_capacity		= 1024u * 1024u;
	constexpr static uint64 k_gpu_resource_capacity = 2048u;
	constexpr static uint64 k_gpu_sampler_capacity	= 2048u;

	void descriptor_manager::reset_gpu_heaps()
	{
		mp_samp_gpu_heap.get_cpu()->free_all();
		mp_srv_gpu_heap.get_cpu()->free_all();
	}

	void descriptor_manager::bind_gpu_heaps(rhi_commandlist& commandlist)
	{
		// bind the descriptor heaps of THIS cpu frame
		commandlist.set_descriptorheaps({ mp_samp_gpu_heap.get_cpu(), mp_srv_gpu_heap.get_cpu() });
	}

	descriptor_manager::descriptor_manager(rhi_device& device)
		: mp_samp_gpu_heap{ renderer_backend::get_instance() }
		, mp_srv_gpu_heap{ renderer_backend::get_instance() }
	{
		using namespace influx::graphics;
		rhi_descheap::create_args create_args{};

		// CPU heaps:
		{	
			create_args.m_shader_visible = false;
			create_args.m_capacity = k_cpu_heap_capacity;

			create_args.m_type = e_descriptor_heap_type::rtv;
			mp_rtv_heap = device.create_descriptor_heap(create_args);
			create_args.m_type = e_descriptor_heap_type::dsv;
			mp_dsv_heap = device.create_descriptor_heap(create_args);
			create_args.m_type = e_descriptor_heap_type::rsc;
			mp_srv_heap = device.create_descriptor_heap(create_args);
			create_args.m_type = e_descriptor_heap_type::sampler;
			mp_sampler_heap = device.create_descriptor_heap(create_args);
		}

		// GPU heaps
		{
			create_args.m_shader_visible = true;

			for (uint32 i = 0u; i < k_num_inflight_max; ++i)
			{
				create_args.m_type = e_descriptor_heap_type::sampler;
				create_args.m_capacity = k_gpu_sampler_capacity;
				mp_samp_gpu_heap.get_at_index(i)  = device.create_descriptor_heap(create_args);
				create_args.m_type = e_descriptor_heap_type::rsc;
				create_args.m_capacity = k_gpu_resource_capacity;
				mp_srv_gpu_heap.get_at_index(i) = device.create_descriptor_heap(create_args);
			}
		}
	}

	descriptor_manager::~descriptor_manager()
	{
		delete mp_rtv_heap;
		delete mp_dsv_heap;
		delete mp_srv_heap;
		delete mp_sampler_heap;
		for (uint32 i = 0u; i < k_num_inflight_max; ++i)
		{
			delete mp_samp_gpu_heap.get_at_index(i);
			delete mp_srv_gpu_heap.get_at_index(i);
		}
	}

	rhi_descriptor descriptor_manager::create_rtv(rhi_device& device, rhi_resource& resource)
	{
		rhi_descriptor cpu_handle = mp_rtv_heap->allocate_cpu().get();
		device.create_rtv(cpu_handle, &resource);
		return cpu_handle;
	}
	rhi_descriptor descriptor_manager::create_dsv(rhi_device& device, rhi_resource& resource)
	{
		rhi_descriptor cpu_handle = mp_dsv_heap->allocate_cpu().get();
		device.create_dsv(cpu_handle, &resource);
		return cpu_handle;
	}
	rhi_descriptor descriptor_manager::create_srv(rhi_device& device, rhi_resource& resource)
	{
		rhi_descriptor cpu_handle = mp_srv_heap->allocate_cpu().get();
		device.create_texture_srv(cpu_handle, &resource);
		return cpu_handle;
	}
	rhi_descriptor descriptor_manager::create_buffer_srv(rhi_device& device, rhi_resource& resource)
	{
		rhi_descriptor cpu_handle = mp_srv_heap->allocate_cpu().get();
		device.create_buffer_srv(cpu_handle, &resource);
		return cpu_handle;
	}
	rhi_descriptor descriptor_manager::create_sampler(rhi_device& device)
	{
		rhi_descriptor cpu_handle = mp_sampler_heap->allocate_cpu().get();
		device.create_sampler_view(cpu_handle, nullptr);
		return cpu_handle;
	}

	rhi_descriptor_range descriptor_manager::stage(rhi_device& device, const vector<rhi_descriptor>& cpu_descriptors)
	{
		rhi_descriptor_range gpu_range{};
		const auto& heap_this_frame = mp_srv_gpu_heap.get_cpu();

		// foreach cpu_descriptor in our list...
		for (size_t i = 0u; i < cpu_descriptors.size(); ++i)
		{
			// allocate a gpu descriptor...	
			rhi_descriptor_id desc_id		= heap_this_frame->allocate().get();
			rhi_descriptor gpu_descriptor	= heap_this_frame->get_gpu(desc_id).get();
			rhi_descriptor cpu_descriptor	= heap_this_frame->get_cpu(desc_id).get();

			// set the first gpu handles as the gpu_range base
			if (i == 0u)
			{
				gpu_range.m_heap_index = desc_id;
				gpu_range.m_start = gpu_descriptor;
			}

			// copy each source cpu descriptor into the cpu descriptor that's associated with our gpu descriptor
			device.copy_descriptors(cpu_descriptors[i], cpu_descriptor, rhi_descheap_type::rsc);
			gpu_range.m_num_descriptors++;
		}

		return gpu_range;
	}

	rhi_descriptor_range descriptor_manager::stage(rhi_device& device, const rhi_descriptor& cpu_descriptor)
	{
		vector<rhi_descriptor> handles{ cpu_descriptor };
		return stage(device, handles);
	}

	rhi_descriptor_range descriptor_manager::stage(rhi_device& device, const vector<texture2D*>& textures)
	{
		vector<rhi_descriptor> cpu_handles{};
		cpu_handles.reserve(textures.size());
		for (size_t i = 0u; i < textures.size(); ++i)
		{
			influx_assert(textures[i] != nullptr);
			influx_assert(textures[i]->get_srv().get() != nullptr);

			cpu_handles.push_back(textures[i]->get_srv().get());
		}

		return stage(device, cpu_handles);
	}

	rhi_descriptor_range descriptor_manager::stage(rhi_device& device, texture2D* texture)
	{
		vector<renderer::texture2D*> textures{ texture };
		return stage(device, textures);
	}

	rhi_descriptor_range descriptor_manager::stage_sampler(rhi_device& device, rhi_descriptor handle)
	{
		return stage_samplers(device, { handle });
	}

	rhi_descriptor_range descriptor_manager::stage_samplers(rhi_device& device, const vector<rhi_descriptor>& samplers)
	{
		const auto& heap_this_frame = mp_samp_gpu_heap.get_cpu();

		rhi_descriptor_range gpu_range{};
		for (size_t i = 0u; i < samplers.size(); ++i)
		{
			// allocate a gpu descriptor
			rhi_descriptor_id desc_id = heap_this_frame->allocate().get();
			rhi_descriptor gpu_handle = heap_this_frame->get_gpu(desc_id).get();
			rhi_descriptor cpu_handle = heap_this_frame->get_cpu(desc_id).get();
			gpu_range.m_num_descriptors++;

			// set the first gpu handles as the gpu_range base
			if (gpu_range.m_start == nullptr || i == 0u)
			{
				gpu_range.m_heap_index = desc_id;
				gpu_range.m_start = gpu_handle;
			}

			// copy the cpu descriptor into the gpu-visible descriptor
			device.copy_descriptors(samplers[i], cpu_handle, rhi_descheap_type::sampler);
		}

		return gpu_range;
	}

	void descriptor_manager::cleanup_rtv(rhi_descriptor rtv)
	{
		mp_rtv_heap->free(rtv).get();
	}

	void descriptor_manager::cleanup_dsv(rhi_descriptor dsv)
	{
		mp_dsv_heap->free(dsv).get();
	}

	void descriptor_manager::cleanup_srv(rhi_descriptor srv)
	{
		mp_srv_heap->free(srv).get();
	}
}