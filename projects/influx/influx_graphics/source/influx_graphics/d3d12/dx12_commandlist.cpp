#include "graphics_pch.h"
#include "influx_graphics/d3d12/dx12_commandlist.h"
#include "dx12_headers.h"

#include "influx_graphics/d3d12/dx12_conversion.h"

#include "influx_graphics/d3d12/dx12_pipeline.h"
#include "influx_graphics/d3d12/dx12_allocator.h"
#include "influx_graphics/d3d12/dx12_resource_views.h"
#include "influx_graphics/d3d12/dx12_resource.h"
#include "influx_graphics/d3d12/dx12_rootsignature.h"
#include "influx_graphics/d3d12/dx12_descriptorheap.h"

namespace influx::graphics
{
	dx12_commandlist::dx12_commandlist(ID3D12GraphicsCommandList* commandlist)
	{
		mp_native = mpdx_commandlist = mpdx_graphics_commandlist = commandlist;
	}

	void dx12_commandlist::start(command_allocator* allocator, pipeline* init_state)
	{
		ID3D12PipelineState* dxpipeline = (init_state ? init_state->get_native<ID3D12PipelineState>() : nullptr);
		ID3D12CommandAllocator* dxallocator = allocator->get_native<ID3D12CommandAllocator>();

		// reset allocator
		dxallocator->Reset();

		// reset commandlist
		mpdx_graphics_commandlist->Reset(dxallocator, dxpipeline);
	}

	void dx12_commandlist::draw_instanced(const draw_instanced_args& args)
	{
		mpdx_graphics_commandlist->DrawInstanced(
			args.m_num_vertices_per_instance,
			args.m_num_instances,
			args.m_start_vertex,
			args.m_start_instance);
	}

	void dx12_commandlist::draw_indexed(const draw_indexed_args& args)
	{
		mpdx_graphics_commandlist->DrawIndexedInstanced(
			args.m_num_indexes_per_instance,
			args.m_num_instances,
			args.m_start_index,
			args.m_start_vertex,
			args.m_start_instance);
	}

	void dx12_commandlist::set_indexbuffer(resource* index_buffer)
	{
		auto dxresource = index_buffer->get_native<ID3D12Resource>();

		D3D12_INDEX_BUFFER_VIEW ib_view{};
		ib_view.BufferLocation = dxresource->GetGPUVirtualAddress();
		ib_view.SizeInBytes = (uint32)index_buffer->get_bytesize();
		ib_view.Format = convert(index_buffer->get_format());

		mpdx_graphics_commandlist->IASetIndexBuffer(&ib_view);
	}

	void dx12_commandlist::set_vertexbuffer(resource* vertex_buffer)
	{
		auto dxresource = vertex_buffer->get_native<ID3D12Resource>();

		D3D12_VERTEX_BUFFER_VIEW vb_view{};
		vb_view.BufferLocation = dxresource->GetGPUVirtualAddress();
		vb_view.SizeInBytes = (uint32)vertex_buffer->get_bytesize();
		vb_view.StrideInBytes = (uint32)vertex_buffer->get_bytestride();

		mpdx_graphics_commandlist->IASetVertexBuffers(0u, 1u, &vb_view);
	}

	void dx12_commandlist::clear_rtv(render_target_view* view, const math::vectorf4& clear_value)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE* cpu_handle = view->get_native<D3D12_CPU_DESCRIPTOR_HANDLE>();
		mpdx_graphics_commandlist->ClearRenderTargetView(*cpu_handle, clear_value.data(), 0u, nullptr);
	}

	void dx12_commandlist::transition_resource(resource* resource, e_resource_state before, e_resource_state after)
	{
		auto dxresource = resource->get_native<ID3D12Resource>();
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(dxresource, convert(before), convert(after));
		mpdx_graphics_commandlist->ResourceBarrier(1u, &barrier);
	}

	void dx12_commandlist::copy_resource(resource* source, resource* dest)
	{
		auto dxsource = source->get_native<ID3D12Resource>();
		auto dxdest = dest->get_native<ID3D12Resource>();

		mpdx_graphics_commandlist->CopyResource(dxdest, dxsource);
	}

	void dx12_commandlist::set(descriptor_heap* heap)
	{
		auto dxheap = heap->get_native<ID3D12DescriptorHeap>();
		mpdx_graphics_commandlist->SetDescriptorHeaps(1u, &dxheap);
	}

	void dx12_commandlist::set(render_target_view* rtv)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE* rtv_handle = rtv->get_native<D3D12_CPU_DESCRIPTOR_HANDLE>();
		mpdx_graphics_commandlist->OMSetRenderTargets(1u, rtv_handle, FALSE, nullptr);
	}

	void dx12_commandlist::set(rootsignature* rootsig)
	{
		auto dxrootsig = rootsig->get_native<ID3D12RootSignature>();
		mpdx_graphics_commandlist->SetGraphicsRootSignature(dxrootsig);
	}

	void dx12_commandlist::set(pipeline* pipeline)
	{
		auto dxpipeline = pipeline->get_native<ID3D12PipelineState>();
		mpdx_graphics_commandlist->SetPipelineState(dxpipeline);
	}

	void dx12_commandlist::set(const viewport& viewport)
	{
		D3D12_VIEWPORT dxviewport{};
		dxviewport.TopLeftX = viewport.m_left;
		dxviewport.TopLeftY = viewport.m_top;
		dxviewport.Width = viewport.m_width;
		dxviewport.Height = viewport.m_height;
		dxviewport.MinDepth = viewport.m_depth_min;
		dxviewport.MaxDepth = viewport.m_depth_max;
		mpdx_graphics_commandlist->RSSetViewports(1u, &dxviewport);
	}

	void dx12_commandlist::set(const rect& rect)
	{
		D3D12_RECT dxrect{};
		dxrect.left = rect.m_left;
		dxrect.top = rect.m_top;
		dxrect.right = rect.m_right;
		dxrect.bottom = rect.m_bottom;
		mpdx_graphics_commandlist->RSSetScissorRects(1u, &dxrect);
	}

	void dx12_commandlist::set(e_primitive_topology topo)
	{
		mpdx_graphics_commandlist->IASetPrimitiveTopology(convert(topo));
	}

	void dx12_commandlist::end()
	{
		mpdx_graphics_commandlist->Close();
	}
}