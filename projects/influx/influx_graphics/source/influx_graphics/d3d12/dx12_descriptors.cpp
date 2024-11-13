#include "graphics_pch.h"
#include "influx_graphics/d3d12/dx12_descriptors.h"
#include "dx12_headers.h"

namespace influx::graphics
{
	dx12_descriptor_heap::dx12_descriptor_heap(const descriptor_heap::create_args& args, 
		ID3D12DescriptorHeap* dxheap, uint64 descriptor_stride)
		: descriptor_heap(args)
		, m_descriptor_stride{descriptor_stride}
	{
		mp_native = mpdx_heap = dxheap;
		set_releasable(mpdx_heap);

		clear_gpu();
		clear_cpu();
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

	void dx12_descriptor_heap::free_cpu(uint32 at_index)
	{
		free_cpu(index_to_cpu_handle(at_index));
	}

	void dx12_descriptor_heap::free_gpu(uint32 at_index)
	{
		free_gpu(index_to_gpu_handle(at_index));
	}

	uint32 dx12_descriptor_heap::get_heap_index_cpu(descriptor_handle handle) const
	{
		return cpu_handle_to_index(handle);
	}

	uint32 dx12_descriptor_heap::get_heap_index_gpu(descriptor_handle handle) const
	{
		return gpu_handle_to_index(handle);
	}

	void dx12_descriptor_heap::free_all_cpu()
	{
		clear_cpu();
	}

	void dx12_descriptor_heap::free_all_gpu()
	{
		clear_gpu();
	}

	void dx12_descriptor_heap::release()
	{
		mpdx_heap->Release();
	}

	void dx12_descriptor_heap::clear_cpu()
	{
		m_freelist_cpu.clear();
		D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = mpdx_heap->GetCPUDescriptorHandleForHeapStart();
		for (size_t i = 0; i < get_capacity(); ++i)
		{
			m_freelist_cpu.push_back(reinterpret_cast<void*>(cpu_handle.ptr + (i * m_descriptor_stride)));
		}
	}

	void dx12_descriptor_heap::clear_gpu()
	{
		m_freelist_gpu.clear();

		if (m_create_args.m_shader_visible)
		{
			D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = mpdx_heap->GetGPUDescriptorHandleForHeapStart();
			for (size_t i = 0; i < get_capacity(); ++i)
			{
				m_freelist_gpu.push_back(reinterpret_cast<void*>(gpu_handle.ptr + (i * m_descriptor_stride)));
			}
		}
	}

	uint32 dx12_descriptor_heap::gpu_handle_to_index(descriptor_handle handle) const
	{
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_base = mpdx_heap->GetGPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle{};
		gpu_handle.ptr = reinterpret_cast<SIZE_T>(handle);

		return (uint32)(gpu_handle.ptr - gpu_base.ptr);
	}

	uint32 dx12_descriptor_heap::cpu_handle_to_index(descriptor_handle handle) const
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cpu_base = mpdx_heap->GetCPUDescriptorHandleForHeapStart();
		D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle{};
		cpu_handle.ptr = reinterpret_cast<SIZE_T>(handle);

		return (uint32)(cpu_handle.ptr - cpu_base.ptr);
	}

	descriptor_handle dx12_descriptor_heap::index_to_gpu_handle(uint32 index) const
	{
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = mpdx_heap->GetGPUDescriptorHandleForHeapStart();
		gpu_handle.ptr += index * m_descriptor_stride;
		return reinterpret_cast<descriptor_handle>(gpu_handle.ptr);
	}

	descriptor_handle dx12_descriptor_heap::index_to_cpu_handle(uint32 index) const
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = mpdx_heap->GetCPUDescriptorHandleForHeapStart();
		cpu_handle.ptr += index * m_descriptor_stride;
		return reinterpret_cast<descriptor_handle>(cpu_handle.ptr);
	}

	dx12_render_target_view::dx12_render_target_view(D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, const resource_info& res_info)
		: render_target_view(reinterpret_cast<descriptor_handle>(cpu_handle.ptr), nullptr, res_info)
		, m_dx_cpu_handle{ cpu_handle }
	{
		mp_native = &m_dx_cpu_handle;
	}

	dx12_depth_stencil_view::dx12_depth_stencil_view(D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle)
		: depth_stencil_view(reinterpret_cast<descriptor_handle>(cpu_handle.ptr), nullptr)
		, m_dx_cpu_handle{ cpu_handle }
	{
		mp_native = &m_dx_cpu_handle;
	}

	dx12_vertex_buffer_view::dx12_vertex_buffer_view(D3D12_VERTEX_BUFFER_VIEW vb_view)
		: vertex_buffer_view(reinterpret_cast<descriptor_handle>(vb_view.BufferLocation), nullptr)
		, m_dx_vbv{ vb_view }
	{
		mp_native = &m_dx_vbv;
	}

	dx12_index_buffer_view::dx12_index_buffer_view(D3D12_INDEX_BUFFER_VIEW index_view)
		: index_buffer_view(reinterpret_cast<descriptor_handle>(index_view.BufferLocation), nullptr)
		, m_dx_ibv{ index_view }
	{
		mp_native = &m_dx_ibv;
	}

	dx12_sampler_view::dx12_sampler_view(D3D12_CPU_DESCRIPTOR_HANDLE descriptor)
		: sampler_view(reinterpret_cast<descriptor_handle>(descriptor.ptr), nullptr)
		, m_dx_descriptor_handle{ descriptor }
	{
		mp_native = &m_dx_descriptor_handle;
	}

	dx12_shader_resource_view::dx12_shader_resource_view(D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle)
		: shader_resource_view(
			reinterpret_cast<descriptor_handle>(cpu_handle.ptr),
			reinterpret_cast<descriptor_handle>(gpu_handle.ptr))
		, m_dx_gpu_handle{ gpu_handle }
		, m_dx_cpu_handle{ cpu_handle }
	{
		mp_native = &m_dx_gpu_handle;
	}
}