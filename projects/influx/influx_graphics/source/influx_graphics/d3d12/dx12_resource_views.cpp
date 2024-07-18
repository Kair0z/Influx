#include "graphics_pch.h"
#include "influx_graphics/d3d12/dx12_resource_views.h"
#include "dx12_headers.h"

namespace influx::graphics
{
	dx12_render_target_view::dx12_render_target_view(D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle)
		: render_target_view( reinterpret_cast<descriptor_handle>(cpu_handle.ptr), nullptr )
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
		: vertex_buffer_view( reinterpret_cast<descriptor_handle>(vb_view.BufferLocation), nullptr )
		, m_dx_vbv{vb_view}
	{
		mp_native = &m_dx_vbv;
	}

	dx12_index_buffer_view::dx12_index_buffer_view(D3D12_INDEX_BUFFER_VIEW index_view)
		: index_buffer_view( reinterpret_cast<descriptor_handle>(index_view.BufferLocation), nullptr )
		, m_dx_ibv{ index_view }
	{
		mp_native = &m_dx_ibv;
	}

	dx12_sampler_view::dx12_sampler_view(D3D12_CPU_DESCRIPTOR_HANDLE descriptor)
		: sampler_view(reinterpret_cast<descriptor_handle>(descriptor.ptr), nullptr )
		, m_dx_descriptor_handle{ descriptor }
	{
		mp_native = &m_dx_descriptor_handle;
	}

	dx12_input_resource_view::dx12_input_resource_view(D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle)
		: input_resource_view(
			reinterpret_cast<descriptor_handle>(cpu_handle.ptr),
			reinterpret_cast<descriptor_handle>(gpu_handle.ptr))
		, m_dx_gpu_handle{ gpu_handle }
		, m_dx_cpu_handle{ cpu_handle }
	{
		mp_native = &m_dx_gpu_handle;
	}
}