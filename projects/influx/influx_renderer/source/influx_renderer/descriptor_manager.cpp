#include "renderer_pch.h"
#include "descriptor_manager.h"

#include "influx_graphics/device.h"
#include "influx_graphics/descriptorheap.h"

namespace influx::renderer
{
	// https://learn.microsoft.com/en-us/windows/win32/direct3d12/hardware-support
	constexpr static uint64 k_max_num_rtvs = 4u;
	constexpr static uint64 k_max_num_srvs = 512u;
	constexpr static uint64 k_max_num_samplers = 2048u;
	constexpr static uint64 k_max_num_dsvs = 64u;

	descriptor_manager::descriptor_manager(graphics::device* device)
		: m_samp_heap{}
		, mp_dsv_heap{}
		, mp_rtv_heap{}
		, m_srv_heap{}
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
		create_args.m_type = e_descriptor_heap_type::srv;
		create_args.m_capacity = k_max_num_srvs;
		create_args.m_shader_visible = false;
		m_srv_heap.mp_cpu_heap = device->create_descriptor_heap(create_args);

		create_args.m_capacity = 128u;
		create_args.m_shader_visible = true;
		m_srv_heap.mp_online_heap = device->create_descriptor_heap(create_args);

		// sampler heap
		create_args.m_type = e_descriptor_heap_type::sampler;
		create_args.m_capacity = k_max_num_samplers;
		create_args.m_shader_visible = false;
		m_samp_heap.mp_cpu_heap = device->create_descriptor_heap(create_args);

		create_args.m_capacity = k_max_num_samplers;
		create_args.m_shader_visible = true;
		m_samp_heap.mp_online_heap = device->create_descriptor_heap(create_args);
	}

	descriptor_manager::~descriptor_manager()
	{
		delete mp_rtv_heap;
		delete m_samp_heap.mp_cpu_heap;
		delete m_samp_heap.mp_online_heap;
		delete mp_dsv_heap;
		delete m_srv_heap.mp_cpu_heap;
		delete m_srv_heap.mp_online_heap;
	}

	graphics::descriptor_heap* descriptor_manager::get_rtv_heap() const
	{
		return get_heap(graphics::e_descriptor_heap_type::rtv);
	}

	graphics::descriptor_heap* descriptor_manager::get_samp_heap() const
	{
		return get_heap(graphics::e_descriptor_heap_type::sampler);
	}

	graphics::descriptor_heap* descriptor_manager::get_srv_heap() const
	{
		return get_heap(graphics::e_descriptor_heap_type::srv);
	}

	graphics::descriptor_heap* descriptor_manager::get_dsv_heap() const
	{
		return get_heap(graphics::e_descriptor_heap_type::dsv);
	}

	graphics::descriptor_heap* descriptor_manager::get_heap(graphics::e_descriptor_heap_type type) const
	{
		switch (type)
		{
		case graphics::e_descriptor_heap_type::dsv: return mp_dsv_heap;
		case graphics::e_descriptor_heap_type::srv: return m_srv_heap.mp_cpu_heap;
		case graphics::e_descriptor_heap_type::rtv: return mp_rtv_heap;
		case graphics::e_descriptor_heap_type::sampler: return m_samp_heap.mp_cpu_heap;
		default:
			influx_assert(false);
			return nullptr;
		}
	}

	graphics::descriptor_handle descriptor_manager::allocate_cpu(graphics::e_descriptor_heap_type type)
	{
		return get_heap(type)->allocate_cpu();
	}

	graphics::descriptor_handle descriptor_manager::allocate_gpu(graphics::e_descriptor_heap_type type)
	{
		return get_heap(type)->allocate_gpu();
	}

	void descriptor_manager::free_cpu(graphics::descriptor_handle handle, graphics::e_descriptor_heap_type type)
	{
		get_heap(type)->free_cpu(handle);
	}

	void descriptor_manager::free_gpu(graphics::descriptor_handle handle, graphics::e_descriptor_heap_type type)
	{
		get_heap(type)->free_gpu(handle);
	}
}