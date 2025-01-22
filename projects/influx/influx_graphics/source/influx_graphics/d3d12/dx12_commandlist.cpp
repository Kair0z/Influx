#include "graphics_pch.h"
#include "influx_graphics/d3d12/dx12_commandlist.h"
#include "dx12_headers.h"

// influx::graphics
#include "influx_graphics/d3d12/dx12_device.h"
#include "influx_graphics/d3d12/dx12_conversion.h"
#include "influx_graphics/d3d12/dx12_pipeline.h"
#include "influx_graphics/d3d12/dx12_descriptors.h"
#include "influx_graphics/d3d12/dx12_resource.h"
#include "influx_graphics/d3d12/dx12_rootsignature.h"

namespace influx::graphics
{
	dx12_commandlist::dx12_commandlist(ID3D12GraphicsCommandList* commandlist, ID3D12CommandAllocator* allocator)
	{
		mp_native = mpdx_commandlist = mpdx_graphics_commandlist = commandlist;
		mpdx_allocator = allocator;
	}

	void dx12_commandlist::start_impl(device* device, pipeline* init_state)
	{
		dx12_device* dxdevice = ((dx12_device*)device);

		if (mpdx_allocator != nullptr)
		{
			free_allocator(dxdevice);
		}

		mpdx_allocator = obtain_allocator(dxdevice);

		ID3D12PipelineState* dxpipeline = (init_state ? init_state->get_native<ID3D12PipelineState>() : nullptr);
		mpdx_graphics_commandlist->Reset(mpdx_allocator, dxpipeline);
	}

	bool g_current_render_pass = true;
	void dx12_commandlist::renderpass_begin(const renderpass_args& args)
	{
		ID3D12GraphicsCommandList7* gfx_commandlist7 = nullptr;
		HRESULT res = mpdx_graphics_commandlist->QueryInterface(&gfx_commandlist7);

		if (!args.m_legacy && res == S_OK && gfx_commandlist7 != nullptr)
		{
			vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC> rtvs{};
			for (uint64 i = 0u; i < args.m_color_attachments.size(); ++i)
			{
				rtvs.push_back({});
				rtvs[i].cpuDescriptor.ptr = (SIZE_T)args.m_color_attachments[i].m_rtv_descriptor;
				rtvs[i].BeginningAccess = translate(args.m_color_attachments[i].m_load);
				rtvs[i].EndingAccess = translate(args.m_color_attachments[i].m_store);
				rtvs[i].BeginningAccess.Clear.ClearValue.Color[1] = 1.0f;
			}

			D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* dsv = nullptr;
			if (false)
			{
				static D3D12_RENDER_PASS_DEPTH_STENCIL_DESC dsv_desc{};
				dsv_desc.cpuDescriptor;
				dsv_desc.DepthBeginningAccess = translate(args.m_depth_attachment.m_depth_load);
				dsv_desc.StencilBeginningAccess = translate(args.m_depth_attachment.m_stencil_load);
				dsv_desc.DepthEndingAccess = translate(args.m_depth_attachment.m_depth_store);
				dsv_desc.StencilEndingAccess = translate(args.m_depth_attachment.m_stencil_store);
				dsv_desc.StencilBeginningAccess.Clear.ClearValue.DepthStencil.Stencil = args.m_depth_attachment.m_stencil_clear;
				dsv_desc.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth = args.m_depth_attachment.m_depth_clear;
				dsv = &dsv_desc;
			}
			
			D3D12_RENDER_PASS_FLAGS flags = translate(args.m_flags);
			gfx_commandlist7->BeginRenderPass((uint32)rtvs.size(), rtvs.data(), dsv, flags);

			g_current_render_pass = true;
		}
		else
		{
			// legacy / manual render passes...
			
		}
	}

	void dx12_commandlist::renderpass_end()
	{
		ID3D12GraphicsCommandList7* gfx_commandlist7 = nullptr;
		HRESULT res = mpdx_graphics_commandlist->QueryInterface(&gfx_commandlist7);
		if (g_current_render_pass && res == S_OK && gfx_commandlist7 != nullptr)
		{
			gfx_commandlist7->EndRenderPass();
			g_current_render_pass = false;
		}
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

	void dx12_commandlist::set_constants(uint32 param_index, uint32 num_dwords, void* source_data)
	{
		mpdx_graphics_commandlist->SetGraphicsRoot32BitConstants(
			param_index,
			num_dwords,
			source_data, 0u);
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

	void dx12_commandlist::clear_rtv(descriptor_handle cpu_handle, const math::vectorf4& clear)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE dx12_handle{};
		dx12_handle.ptr = (SIZE_T)cpu_handle;
		mpdx_graphics_commandlist->ClearRenderTargetView(dx12_handle, clear.data(), 0u, nullptr);
	}

	void dx12_commandlist::clear_rtv(render_target_view* view, const math::vectorf4& clear_value)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE* cpu_handle = view->get_native<D3D12_CPU_DESCRIPTOR_HANDLE>();
		clear_rtv(reinterpret_cast<descriptor_handle>(cpu_handle->ptr), clear_value);
	}

	void dx12_commandlist::clear_dsv(depth_stencil_view* view, float clear_depth, uint32 clear_stencil)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE* cpu_handle = view->get_native<D3D12_CPU_DESCRIPTOR_HANDLE>();
		mpdx_graphics_commandlist->ClearDepthStencilView(*cpu_handle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, clear_depth, clear_stencil, 0u, nullptr);
	}

	void dx12_commandlist::transition_resource(resource* resource, e_resource_state before, e_resource_state after)
	{
		auto dxresource = resource->get_native<ID3D12Resource>();
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(dxresource, convert(before), convert(after));
		mpdx_graphics_commandlist->ResourceBarrier(1u, &barrier);
	}

	void dx12_commandlist::buffer_barrier(resource* resource, e_resource_state before, e_resource_state after)
	{
		D3D12_BUFFER_BARRIER barrier{};
		barrier.SyncBefore = get_barrier_sync(before);
		barrier.SyncAfter = get_barrier_sync(after);
		barrier.AccessBefore = get_barrier_access(before);
		barrier.AccessAfter = get_barrier_access(after);
		barrier.Offset = 0u;
		barrier.Size = UINT64_MAX;
		barrier.pResource = resource->get_native<ID3D12Resource>();
		m_buffer_barriers.push_back(barrier);
	}

	void dx12_commandlist::texture_barrier(resource* resource, e_resource_state before, e_resource_state after)
	{
		D3D12_TEXTURE_BARRIER barrier{};
		barrier.SyncBefore = get_barrier_sync(before);
		barrier.SyncAfter= get_barrier_sync(after);
		barrier.AccessBefore = get_barrier_access(before);
		barrier.AccessAfter = get_barrier_access(after);
		barrier.LayoutBefore = get_barrier_layout(before);
		barrier.LayoutAfter = get_barrier_layout(after);
		barrier.pResource = resource->get_native<ID3D12Resource>();
		barrier.Subresources = CD3DX12_BARRIER_SUBRESOURCE_RANGE(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
		if (has_any_flag(before, e_resource_state::discard)) barrier.Flags = D3D12_TEXTURE_BARRIER_FLAG_DISCARD;
		m_texture_barriers.push_back(barrier);
	}

	void dx12_commandlist::global_barrier(e_resource_state before, e_resource_state after)
	{
		D3D12_GLOBAL_BARRIER barrier{};
		barrier.SyncBefore = get_barrier_sync(before);
		barrier.SyncAfter = get_barrier_sync(after);
		barrier.AccessBefore = get_barrier_access(before);
		barrier.AccessAfter = get_barrier_access(after);
		m_global_barriers.push_back(barrier);
	}

	void dx12_commandlist::flush_barriers()
	{
		vector<D3D12_BARRIER_GROUP> barrier_groups{};
		barrier_groups.reserve(3);

		ID3D12GraphicsCommandList7* cmdlist7 = nullptr;
		if (mpdx_commandlist->QueryInterface(&cmdlist7) != S_OK)
		{
			// OY
			influx_assert(false);
		}

		if (!m_texture_barriers.empty())
		{
			barrier_groups.push_back(CD3DX12_BARRIER_GROUP((uint32)m_texture_barriers.size(), m_texture_barriers.data()));
		}

		if (!m_buffer_barriers.empty())
		{
			barrier_groups.push_back(CD3DX12_BARRIER_GROUP((uint32)m_buffer_barriers.size(), m_buffer_barriers.data()));
		}

		if (!m_global_barriers.empty())
		{
			barrier_groups.push_back(CD3DX12_BARRIER_GROUP((uint32)m_global_barriers.size(), m_global_barriers.data()));
		}

		if (!barrier_groups.empty())
		{
			cmdlist7->Barrier((uint32)barrier_groups.size(), barrier_groups.data());
		}

		m_texture_barriers.clear();
		m_buffer_barriers.clear();
		m_global_barriers.clear();
	}

	void dx12_commandlist::copy_resource(resource* source, resource* dest)
	{
		auto dxsource = source->get_native<ID3D12Resource>();
		auto dxdest = dest->get_native<ID3D12Resource>();

		mpdx_graphics_commandlist->CopyResource(dxdest, dxsource);
	}

	void dx12_commandlist::copy_texture(resource* src, resource* dest, 
		const copy_texture_args& args)
	{
		CD3DX12_TEXTURE_COPY_LOCATION src_loc{ src->get_native<ID3D12Resource>() };
		src_loc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src_loc.PlacedFootprint.Offset = 0;
		src_loc.PlacedFootprint.Footprint.Width = dest->get_width();
		src_loc.PlacedFootprint.Footprint.Height = dest->get_height();
		src_loc.PlacedFootprint.Footprint.Depth = 1;
		src_loc.PlacedFootprint.Footprint.Format = convert(dest->get_format());
		src_loc.PlacedFootprint.Footprint.RowPitch = dest->get_width() * (uint32)dest->get_bytestride();

		CD3DX12_TEXTURE_COPY_LOCATION dest_loc{ dest->get_native<ID3D12Resource>() };
		
		D3D12_BOX src_box{};
		src_box.left = 0u;
		src_box.right = dest->get_width();
		src_box.bottom = dest->get_height();
		src_box.top = 0u;
		src_box.front = 0u;
		src_box.back = 1u;

		mpdx_graphics_commandlist->CopyTextureRegion(
			&dest_loc, 0u, 0u, 0u,
			&src_loc, &src_box);
	}

	void dx12_commandlist::copy_buffer(resource* src, resource* dest, uint32 bytesize, const copy_buffer_args& args)
	{
		mpdx_graphics_commandlist->CopyBufferRegion(
			dest->get_native<ID3D12Resource>(), args.m_dest_offset,
			src->get_native<ID3D12Resource>(), args.m_src_offset,
			bytesize);
	}

	void dx12_commandlist::set(descriptor_heap* heap)
	{
		auto dxheap = heap->get_native<ID3D12DescriptorHeap>();
		mpdx_graphics_commandlist->SetDescriptorHeaps(1u, &dxheap);
	}

	void dx12_commandlist::set(render_target_view* rtv, depth_stencil_view* dsv)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE* rtv_handle = rtv->get_native<D3D12_CPU_DESCRIPTOR_HANDLE>();
		D3D12_CPU_DESCRIPTOR_HANDLE* dsv_handle = dsv ? dsv->get_native<D3D12_CPU_DESCRIPTOR_HANDLE>() : nullptr;
		mpdx_graphics_commandlist->OMSetRenderTargets(1u, rtv_handle, FALSE, dsv_handle);
	}

	void dx12_commandlist::set(shader_resource_view* srv, uint32 param_idx)
	{
		auto dx12srv = (dx12_shader_resource_view*)srv;

		D3D12_GPU_DESCRIPTOR_HANDLE srv_gpu_handle{};
		srv_gpu_handle.ptr = (size_t)dx12srv->get_gpu_handle();
		mpdx_graphics_commandlist->SetGraphicsRootDescriptorTable(param_idx, srv_gpu_handle);
	}

	void dx12_commandlist::set(const descriptor_range& gpu_range, uint32 param_idx)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle{};
		gpu_handle.ptr = (size_t)gpu_range.m_start;
		mpdx_graphics_commandlist->SetGraphicsRootDescriptorTable(param_idx, gpu_handle);
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

	void dx12_commandlist::release_impl(device*)
	{
		mpdx_graphics_commandlist->Release();
	}

	ID3D12CommandAllocator* dx12_commandlist::obtain_allocator(dx12_device* dxdevice)
	{
		influx_assert(dxdevice);
		mpdx_allocator = dxdevice->new_allocator(D3D12_COMMAND_LIST_TYPE_DIRECT);
		return mpdx_allocator;
	}

	void dx12_commandlist::free_allocator(dx12_device* dxdevice)
	{
		influx_assert(dxdevice);
		dxdevice->free_allocator(D3D12_COMMAND_LIST_TYPE_DIRECT, mpdx_allocator);
		mpdx_allocator = nullptr;
	}
}