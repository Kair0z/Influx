#include "renderer_pch.h"
#include "descriptor_manager.h"

#include "influx_graphics/device.h"
#include "influx_graphics/descriptorheap.h"

namespace influx::renderer
{
	// https://learn.microsoft.com/en-us/windows/win32/direct3d12/hardware-support
	constexpr static uint64 k_max_num_rtvs = 4u;
	constexpr static uint64 k_max_num_srvs = 4096u;
	constexpr static uint64 k_max_num_samplers = 16u;
	constexpr static uint64 k_max_num_dsvs = 64u;

	descriptor_manager::descriptor_manager(graphics::device* device)
		: mp_cbv_heap{}
		, mp_dsv_heap{}
		, mp_rtv_heap{}
		, mp_sampler_heap{}
	{
		using namespace influx::graphics;

		// rtv heap
		graphics::descriptor_heap::create_args create_args{e_descriptor_heap_type::rtv, k_max_num_rtvs, false };
		mp_rtv_heap = device->create_descriptor_heap(create_args);

		// dsv heap
		create_args.m_capacity = k_max_num_dsvs;
		create_args.m_type = e_descriptor_heap_type::dsv;
		mp_dsv_heap = device->create_descriptor_heap(create_args);

		// srv heap
		create_args.m_capacity = k_max_num_srvs;
		create_args.m_type = e_descriptor_heap_type::srv;
		create_args.m_shader_visible = true;
		mp_cbv_heap = device->create_descriptor_heap(create_args);

		// sampler heap
		create_args.m_capacity = k_max_num_samplers;
		create_args.m_type = e_descriptor_heap_type::sampler;
		create_args.m_shader_visible = true;
		mp_sampler_heap = device->create_descriptor_heap(create_args);
	}

	descriptor_manager::~descriptor_manager()
	{
		delete mp_rtv_heap;
		delete mp_cbv_heap;
		delete mp_dsv_heap;
		delete mp_sampler_heap;
	}

	graphics::descriptor_heap* descriptor_manager::get_rtv_heap() const
	{
		return mp_rtv_heap;
	}
	graphics::descriptor_heap* descriptor_manager::get_samp_heap() const
	{
		return mp_sampler_heap;
	}
	graphics::descriptor_heap* descriptor_manager::get_srv_heap() const
	{
		return mp_cbv_heap;
	}
	graphics::descriptor_heap* descriptor_manager::get_dsv_heap() const
	{
		return mp_dsv_heap;
	}

	vector<descriptor_manager::descriptor_couple> descriptor_manager::allocate_srv(uint64 num_descriptors)
	{
		vector<descriptor_couple> couples{};
		for (uint64 i = 0u; i < num_descriptors; ++i)
		{
			couples[i].m_cpu_handle = mp_cbv_heap->allocate_cpu();
			couples[i].m_gpu_handle = mp_cbv_heap->allocate_gpu();
		}

		m_srv_allocation_buffer.push(couples);
	}

	void descriptor_manager::free_srv(uint64 num_descriptors)
	{
		vector<descriptor_couple> couples{};
		for (uint64 i = 0u; i < num_descriptors; ++i)
		{
			m_srv_allocation_buffer.pop_lockless(couples[i]);
		}

		for (uint64 i = 0u; i < num_descriptors; ++i)
		{
			mp_cbv_heap->free_cpu(couples[i].m_cpu_handle);
			mp_cbv_heap->free_gpu(couples[i].m_gpu_handle);
		}
	}
}