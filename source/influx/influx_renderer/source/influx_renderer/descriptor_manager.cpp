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
	constexpr static uint64 k_max_num_rtvs = 64u;
	constexpr static uint64 k_max_num_srvs = 1024u;
	constexpr static uint64 k_max_num_samplers = 2048u;
	constexpr static uint64 k_max_num_dsvs = 64u;

	void descriptor_manager::bind_gpu_heaps(rhi_commandlist& commandlist)
	{
		commandlist.set_descriptorheaps({ mp_samp_gpu_heap, mp_srv_gpu_heap });
	}

	void descriptor_manager::reset_gpu_heaps()
	{
		mp_srv_gpu_heap->free_all();
		mp_samp_gpu_heap->free_all();
	}

	descriptor_manager::descriptor_manager(rhi_device& device)
	{
		using namespace influx::graphics;
		rhi_descheap::create_args create_args{};

		// CPU heaps:
		{	
			create_args.m_shader_visible = false;

			// rtv heap
			create_args.m_capacity = k_max_num_rtvs;
			create_args.m_type = e_descriptor_heap_type::rtv;
			mp_rtv_heap = device.create_descriptor_heap(create_args);

			// dsv heap
			create_args.m_capacity = k_max_num_dsvs;
			create_args.m_type = e_descriptor_heap_type::dsv;
			mp_dsv_heap = device.create_descriptor_heap(create_args);

			// srv heap
			create_args.m_type = e_descriptor_heap_type::rsc;
			create_args.m_capacity = k_max_num_srvs;
			mp_srv_heap = device.create_descriptor_heap(create_args);

			// sampler heap
			create_args.m_type = e_descriptor_heap_type::sampler;
			create_args.m_capacity = k_max_num_samplers;
			mp_sampler_heap = device.create_descriptor_heap(create_args);
		}

		// GPU heaps
		{
			create_args.m_shader_visible = true;

			// sampler heap
			create_args.m_type = e_descriptor_heap_type::sampler;
			create_args.m_capacity = 8u;
			mp_samp_gpu_heap = device.create_descriptor_heap(create_args);

			// srv heap
			create_args.m_type = e_descriptor_heap_type::rsc;
			create_args.m_capacity = 2048u;
			mp_srv_gpu_heap = device.create_descriptor_heap(create_args);
		}
	}

	descriptor_manager::~descriptor_manager()
	{
		delete mp_rtv_heap;
		delete mp_dsv_heap;
		delete mp_srv_heap;
		delete mp_sampler_heap;
		delete mp_samp_gpu_heap;
		delete mp_srv_gpu_heap;
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

		// foreach cpu_descriptor in our list...
		for (size_t i = 0u; i < cpu_descriptors.size(); ++i)
		{
			// allocate a gpu descriptor...
			rhi_descriptor_id desc_id = mp_srv_gpu_heap->allocate().get();
			rhi_descriptor gpu_descriptor = mp_srv_gpu_heap->get_gpu(desc_id).get();

			gpu_range.m_num_descriptors++;

			// set the first gpu handles as the gpu_range base
			if (gpu_range.m_start == nullptr || i == 0u)
			{
				gpu_range.m_start = gpu_descriptor;
				// gpu_range.m_start_idx = mp_srv_gpu_heap->get_heap_index_gpu(gpu_handle);
			}

			// copy the cpu descriptor into the gpu-visible descriptor
			device.copy_descriptors(cpu_descriptors[i], gpu_descriptor, rhi_descheap_type::rsc);
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
		rhi_descriptor_range gpu_range{};
		for (size_t i = 0u; i < samplers.size(); ++i)
		{
			// allocate a gpu descriptor and 
			rhi_descriptor gpu_handle = mp_samp_gpu_heap->allocate_gpu().get();
			rhi_descriptor cpu_handle = mp_samp_gpu_heap->allocate_cpu().get();
			gpu_range.m_num_descriptors++;

			// set the first gpu handles as the gpu_range base
			if (gpu_range.m_start == nullptr)
			{
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