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

	descriptor_handle dx12_descriptor_heap::allocate()
	{
		descriptor_handle handle = m_freelist.back();
		m_freelist.pop_back();
		return handle;
	}

	void dx12_descriptor_heap::free(descriptor_handle handle)
	{
		m_freelist.push_back(handle);
	}

	void dx12_descriptor_heap::clear()
	{
		m_freelist.clear();

		D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = mpdx_heap->GetCPUDescriptorHandleForHeapStart();
		for (size_t i = 0; i < get_capacity(); ++i)
		{
			m_freelist.push_back(reinterpret_cast<void*>(cpu_handle.ptr + (i * m_descriptor_stride)));
		}
	}
}