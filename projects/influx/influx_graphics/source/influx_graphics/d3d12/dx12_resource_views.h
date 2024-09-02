#pragma once
#include "influx_graphics/resource_views.h"

#include "influx_graphics/d3d12/dx12_headers.h"

namespace influx::graphics
{
	class dx12_render_target_view : public render_target_view
	{
	public:
		dx12_render_target_view(D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, const resource_info& res_info);

	private:
		D3D12_CPU_DESCRIPTOR_HANDLE m_dx_cpu_handle;
	};

	class dx12_depth_stencil_view : public depth_stencil_view
	{
	public:
		dx12_depth_stencil_view(D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle);

	private:
		D3D12_CPU_DESCRIPTOR_HANDLE m_dx_cpu_handle;
	};

	class dx12_vertex_buffer_view : public vertex_buffer_view
	{
	public:
		dx12_vertex_buffer_view(D3D12_VERTEX_BUFFER_VIEW vb_view);

	private:
		D3D12_VERTEX_BUFFER_VIEW m_dx_vbv;
	};

	class dx12_index_buffer_view : public index_buffer_view
	{
	public:
		dx12_index_buffer_view(D3D12_INDEX_BUFFER_VIEW index_view);

	private:
		D3D12_INDEX_BUFFER_VIEW m_dx_ibv;
	};

	class dx12_sampler_view : public sampler_view
	{
	public:
		dx12_sampler_view(D3D12_CPU_DESCRIPTOR_HANDLE descriptor);

	private:
		D3D12_CPU_DESCRIPTOR_HANDLE m_dx_descriptor_handle;
	};

	class dx12_shader_resource_view : public shader_resource_view
	{
	public:
		dx12_shader_resource_view(D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle);

	private:
		D3D12_GPU_DESCRIPTOR_HANDLE m_dx_gpu_handle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_dx_cpu_handle;
	};
}