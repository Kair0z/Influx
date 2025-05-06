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

	void descriptor_manager::start_frame()
	{
		
	}

	void descriptor_manager::start_commandlist(graphics::commandlist* commandlist)
	{
		commandlist->set({ mp_samp_gpu_heap, mp_srv_gpu_heap });
	}

	void descriptor_manager::end_frame()
	{
		mp_srv_gpu_heap->free_all_cpu();
		mp_srv_gpu_heap->free_all_gpu();
		
		mp_samp_gpu_heap->free_all_cpu();
		mp_samp_gpu_heap->free_all_gpu();
	}

	descriptor_manager::descriptor_manager(graphics::device* device)
		: mp_device{ device }
	{
		using namespace influx::graphics;
		graphics::descriptor_heap::create_args create_args{};

		// CPU heaps:
		{	
			create_args.m_shader_visible = false;

			// rtv heap
			create_args.m_capacity = k_max_num_rtvs;
			create_args.m_type = e_descriptor_heap_type::rtv;
			mp_rtv_heap = device->create_descriptor_heap(create_args);

			// dsv heap
			create_args.m_capacity = k_max_num_dsvs;
			create_args.m_type = e_descriptor_heap_type::dsv;
			mp_dsv_heap = device->create_descriptor_heap(create_args);

			// srv heap
			create_args.m_type = e_descriptor_heap_type::srv;
			create_args.m_capacity = k_max_num_srvs;
			mp_srv_heap = device->create_descriptor_heap(create_args);

			// sampler heap
			create_args.m_type = e_descriptor_heap_type::sampler;
			create_args.m_capacity = k_max_num_samplers;
			mp_sampler_heap = device->create_descriptor_heap(create_args);
		}

		// GPU heaps
		{
			create_args.m_shader_visible = true;

			// sampler heap
			create_args.m_type = e_descriptor_heap_type::sampler;
			create_args.m_capacity = 8u;
			mp_samp_gpu_heap = device->create_descriptor_heap(create_args);

			// srv heap
			create_args.m_type = e_descriptor_heap_type::srv;
			create_args.m_capacity = 2048u;
			mp_srv_gpu_heap = device->create_descriptor_heap(create_args);
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

	graphics::descriptor_handle descriptor_manager::create_rtv(graphics::resource* resource)
	{
		graphics::descriptor_handle cpu_handle = mp_rtv_heap->allocate_cpu().get();
		mp_device->create_rtv(cpu_handle, resource);
		return cpu_handle;
	}

	graphics::descriptor_handle descriptor_manager::create_dsv(graphics::resource* resource)
	{
		graphics::descriptor_handle cpu_handle = mp_dsv_heap->allocate_cpu().get();
		mp_device->create_dsv(cpu_handle, resource);
		return cpu_handle;
	}

	graphics::descriptor_handle descriptor_manager::create_srv(graphics::resource* resource)
	{
		graphics::descriptor_handle cpu_handle = mp_srv_heap->allocate_cpu().get();
		mp_device->create_texture_srv(cpu_handle, resource);
		return cpu_handle;
	}

	graphics::descriptor_handle descriptor_manager::create_buffer_srv(graphics::resource* resource)
	{
		graphics::descriptor_handle cpu_handle = mp_srv_heap->allocate_cpu().get();
		mp_device->create_buffer_srv(cpu_handle, resource);
		return cpu_handle;
	}

	graphics::descriptor_handle descriptor_manager::create_sampler()
	{
		graphics::descriptor_handle cpu_handle = mp_sampler_heap->allocate_cpu().get();
		mp_device->create_sampler_view(cpu_handle, nullptr);
		return cpu_handle;
	}

	graphics::descriptor_range descriptor_manager::stage(const vector<graphics::descriptor_handle>& cpu_descriptors)
	{
		graphics::descriptor_range gpu_range{};
		for (size_t i = 0u; i < cpu_descriptors.size(); ++i)
		{
			// allocate a gpu descriptor and 
			graphics::descriptor_handle gpu_handle = mp_srv_gpu_heap->allocate_gpu().get();
			graphics::descriptor_handle cpu_handle = mp_srv_gpu_heap->allocate_cpu().get();
			gpu_range.m_num_descriptors++;

			// set the first gpu handles as the gpu_range base
			if (gpu_range.m_start == nullptr)
			{
				gpu_range.m_start = gpu_handle;
				gpu_range.m_start_idx = mp_srv_gpu_heap->get_heap_index_gpu(gpu_handle);
			}

			// copy the cpu descriptor into the gpu-visible descriptor
			mp_device->copy_descriptors(cpu_descriptors[i], cpu_handle, graphics::e_descriptor_heap_type::srv);
		}

		return gpu_range;
	}

	graphics::descriptor_range descriptor_manager::stage(const graphics::descriptor_handle& cpu_descriptor)
	{
		vector<graphics::descriptor_handle> handles{ cpu_descriptor };
		return stage(handles);
	}

	graphics::descriptor_range descriptor_manager::stage(const vector<texture2D*>& textures)
	{
		vector<graphics::descriptor_handle> cpu_handles{};
		cpu_handles.reserve(textures.size());
		for (size_t i = 0u; i < textures.size(); ++i)
		{
			influx_assert(textures[i] != nullptr);
			influx_assert(textures[i]->get_srv() != nullptr);

			cpu_handles.push_back(textures[i]->get_srv());
		}

		return stage(cpu_handles);
	}

	graphics::descriptor_range descriptor_manager::stage(texture2D* texture)
	{
		vector<renderer::texture2D*> textures{ texture };
		return stage(textures);
	}

	graphics::descriptor_range descriptor_manager::stage_sampler(graphics::descriptor_handle handle)
	{
		return stage_samplers({ handle });
	}

	graphics::descriptor_range descriptor_manager::stage_samplers(const vector<graphics::descriptor_handle>& samplers)
	{
		graphics::descriptor_range gpu_range{};
		for (size_t i = 0u; i < samplers.size(); ++i)
		{
			// allocate a gpu descriptor and 
			graphics::descriptor_handle gpu_handle = mp_samp_gpu_heap->allocate_gpu().get();
			graphics::descriptor_handle cpu_handle = mp_samp_gpu_heap->allocate_cpu().get();
			gpu_range.m_num_descriptors++;

			// set the first gpu handles as the gpu_range base
			if (gpu_range.m_start == nullptr)
			{
				gpu_range.m_start = gpu_handle;
			}

			// copy the cpu descriptor into the gpu-visible descriptor
			mp_device->copy_descriptors(samplers[i], cpu_handle, graphics::e_descriptor_heap_type::sampler);
		}

		return gpu_range;
	}

	void descriptor_manager::cleanup_rtv(graphics::descriptor_handle rtv)
	{
		mp_rtv_heap->free_cpu(rtv);
	}

	void descriptor_manager::cleanup_dsv(graphics::descriptor_handle dsv)
	{
		mp_dsv_heap->free_cpu(dsv);
	}

	void descriptor_manager::cleanup_srv(graphics::descriptor_handle srv)
	{
		mp_srv_heap->free_cpu(srv);
	}
}