#include "graphics_pch.h"
#include "influx_graphics/d3d12/dx12_descriptorheap.h"
#include "dx12_headers.h"

namespace influx::graphics
{
	dx12_descriptor_heap::dx12_descriptor_heap(const descriptor_heap::create_args& args, 
		ID3D12DescriptorHeap* dxheap, uint64 descriptor_stride)
		: descriptor_heap(args)
		, m_descriptor_stride{descriptor_stride}
	{
		mp_native = mpdx_heap = dxheap;

		clear();
	}

	descriptor_handle dx12_descriptor_heap::allocate_cpu()
	{
		descriptor_handle handle = m_freelist_cpu.front();
		m_freelist_cpu.pop_front();
		return handle;
	}

	descriptor_handle dx12_descriptor_heap::allocate_gpu()
	{
		descriptor_handle handle = m_freelist_gpu.front();
		m_freelist_gpu.pop_front();
		return handle;
	}

	void dx12_descriptor_heap::free_cpu(descriptor_handle handle)
	{
		m_freelist_cpu.push_back(handle);
	}

	void dx12_descriptor_heap::free_gpu(descriptor_handle handle)
	{
		m_freelist_gpu.push_back(handle);
	}

	void dx12_descriptor_heap::clear()
	{
		m_freelist_cpu.clear();
		m_freelist_gpu.clear();

		D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = mpdx_heap->GetCPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = mpdx_heap->GetGPUDescriptorHandleForHeapStart();
		for (size_t i = 0; i < get_capacity(); ++i)
		{
			m_freelist_cpu.push_back(reinterpret_cast<void*>(cpu_handle.ptr + (i * m_descriptor_stride)));
			m_freelist_gpu.push_back(reinterpret_cast<void*>(gpu_handle.ptr + (i * m_descriptor_stride)));
		}
	}
}