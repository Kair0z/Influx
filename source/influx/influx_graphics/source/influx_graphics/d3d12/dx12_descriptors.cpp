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

		m_freelist.reserve(get_capacity());
		for (uint32 i = 0u; i < get_capacity(); ++i)
		{
			m_freelist.push_back({ .m_index = i, .m_is_allocated = false });
		}
	}

	result<descriptor_id> dx12_descriptor_heap::allocate()
	{
		using result_type = result<descriptor_id>;
		for (uint32 i = 0u; i < get_capacity(); ++i)
		{
			if (m_freelist[i].m_is_allocated == false)
			{
				m_freelist[i].m_is_allocated = true;
				return m_freelist[i].m_index;
			}
		}
		return result_type::make_error("error: failed to allocate descriptor because we ran out of available slots!");
	}

	result<> dx12_descriptor_heap::free(descriptor_id handle)
	{
		if (handle >= m_freelist.size())
			return result<>::make_error("error: failed freeing cpu at index out of range!");

		m_freelist[handle].m_is_allocated = false;
		return {};
	}

	result<descriptor_handle> dx12_descriptor_heap::get_cpu(descriptor_id handle) const
	{
		using result_type = result<descriptor_handle>;
		return index_to_cpu_handle(handle);
	}
	result<descriptor_handle> dx12_descriptor_heap::get_gpu(descriptor_id handle) const
	{
		using result_type = result<descriptor_handle>;
		return index_to_gpu_handle(handle);
	}
	result<descriptor_id> dx12_descriptor_heap::get_id(descriptor_handle handle) const
	{
		using result_type = result<descriptor_id>;

		// if found on cpu_heap, return that index
		auto handle_to_index = cpu_handle_to_index(handle);
		if (handle_to_index) return handle_to_index;

		if (m_create_args.m_shader_visible)
		{
			handle_to_index = gpu_handle_to_index(handle);
			if (handle_to_index) return handle_to_index;
		}

		return result_type::make_error("handle does not fall inside this GPU heap!");
	}

	result<> dx12_descriptor_heap::free_all()
	{
		clear();
		return {};
	}

	void dx12_descriptor_heap::release_impl(device*)
	{
		mpdx_heap->Release();
	}

	void dx12_descriptor_heap::clear()
	{
		for (uint32 i = 0u; i < get_capacity(); ++i)
		{
			m_freelist[i].m_is_allocated = false;
		}
	}

	result<uint32> dx12_descriptor_heap::gpu_handle_to_index(descriptor_handle handle) const
	{
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_base = mpdx_heap->GetGPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle{};
		gpu_handle.ptr = reinterpret_cast<SIZE_T>(handle);

		uint32 index = (uint32)(gpu_handle.ptr - gpu_base.ptr) / static_cast<uint32>(m_descriptor_stride);
		if (index >= get_capacity())
			return result<uint32>::make_error("error: handle does not translate to a valid index! (it probably doesn't belong here)");

		return index;
	}

	result<uint32> dx12_descriptor_heap::cpu_handle_to_index(descriptor_handle handle) const
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cpu_base = mpdx_heap->GetCPUDescriptorHandleForHeapStart();
		D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle{};
		cpu_handle.ptr = reinterpret_cast<SIZE_T>(handle);

		uint32 index = (uint32)(cpu_handle.ptr - cpu_base.ptr) / static_cast<uint32>(m_descriptor_stride);
		if (index >= get_capacity())
			return result<uint32>::make_error("error: handle does not translate to a valid index! (it probably doesn't belong here)");

		return index;
	}

	result<descriptor_handle> dx12_descriptor_heap::index_to_gpu_handle(uint32 index) const
	{
		if (index >= get_capacity())
			return result<descriptor_handle>::make_error("error: invalid index! exceeding capacity!");

		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = mpdx_heap->GetGPUDescriptorHandleForHeapStart();
		gpu_handle.ptr += index * m_descriptor_stride;
		return reinterpret_cast<descriptor_handle>(gpu_handle.ptr);
	}

	result<descriptor_handle> dx12_descriptor_heap::index_to_cpu_handle(uint32 index) const
	{
		if (index >= get_capacity())
			return result<descriptor_handle>::make_error("error: invalid index! exceeding capacity!");

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