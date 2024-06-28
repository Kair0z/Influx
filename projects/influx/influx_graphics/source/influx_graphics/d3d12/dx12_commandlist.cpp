#include "graphics_pch.h"
#include "influx_graphics/d3d12/dx12_commandlist.h"
#include "dx12_headers.h"

#include "influx_graphics/d3d12/dx12_conversion.h"

#include "influx_graphics/pipelinestate.h"
#include "influx_graphics/d3d12/dx12_allocator.h"
#include "influx_graphics/d3d12/dx12_resource_views.h"
#include "influx_graphics/d3d12/dx12_resource.h"

namespace influx::graphics
{
	dx12_commandlist::dx12_commandlist(ID3D12GraphicsCommandList* commandlist)
	{
		mp_native = mpdx_commandlist = mpdx_graphics_commandlist = commandlist;
	}

	void dx12_commandlist::start(command_allocator* allocator, pipeline_state* init_state)
	{
		ID3D12PipelineState* dxpipeline = (init_state ? init_state->get_native<ID3D12PipelineState>() : nullptr);
		ID3D12CommandAllocator* dxallocator = allocator->get_native<ID3D12CommandAllocator>();

		// reset allocator
		dxallocator->Reset();

		// reset commandlist
		mpdx_graphics_commandlist->Reset(dxallocator, dxpipeline);
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

	void dx12_commandlist::end()
	{
		mpdx_graphics_commandlist->Close();
	}
}