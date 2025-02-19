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
	const bool g_mute = false;
	#define renderpass_check(command) \
		if (!is_renderpass_valid(command)) \
		{ \
			if (!g_mute) logwar("{} is not allowed inside a renderpass!", #command);\
			return;\
		}\
		
	dx12_commandlist::dx12_commandlist(ID3D12GraphicsCommandList* commandlist, ID3D12CommandAllocator* allocator)
	{
		mp_native = mpdx_commandlist = mpdx_graphics_commandlist = commandlist;
		mpdx_allocator = allocator;
	}

	void dx12_commandlist::start_impl(device* device, detail::base_pipeline* init_state)
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

	void dx12_commandlist::renderpass_begin(const renderpass_args& args)
	{
		if (!is_renderpass_valid(e_command::begin_renderpass))
		{
			if (!g_mute) logwar("renderpass_begin is not allowed inside a renderpass!");
			return;
		}

		ID3D12GraphicsCommandList7* gfx_commandlist7 = nullptr;
		HRESULT res = mpdx_graphics_commandlist->QueryInterface(&gfx_commandlist7);

		if (!args.m_legacy && res == S_OK && gfx_commandlist7 != nullptr)
		{
			vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC> rtvs{};
			for (uint64 i = 0u; i < args.m_color_attachments.size(); ++i)
			{
				const auto& attachment = args.m_color_attachments[i];
				rtvs.push_back({});
				rtvs[i].cpuDescriptor.ptr = (SIZE_T)args.m_color_attachments[i].m_rtv_descriptor;
				rtvs[i].BeginningAccess = translate(args.m_color_attachments[i].m_load);
				rtvs[i].EndingAccess = translate(args.m_color_attachments[i].m_store);

				// load: preserve
				if (attachment.m_load == e_load_op::preserve)
				{
					rtvs[i].BeginningAccess.PreserveLocal.AdditionalHeight = 0u;
					rtvs[i].BeginningAccess.PreserveLocal.AdditionalWidth = 0u;
				}
				
				// load: clear
				if (attachment.m_load == e_load_op::clear)
				{
					rtvs[i].BeginningAccess.Clear.ClearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
					memcpy(rtvs[i].BeginningAccess.Clear.ClearValue.Color, args.m_color_attachments[i].m_clear.m_data, sizeof(FLOAT[4]));
				}
				
				// store: resolve
				if (attachment.m_store == e_store_op::resolve)
				{
					const auto& resolve = attachment.m_resolve;
					rtvs[i].EndingAccess.Resolve.Format = translate(resolve.m_format);
					rtvs[i].EndingAccess.Resolve.pSrcResource = resolve.m_source->get_native<ID3D12Resource>();
					rtvs[i].EndingAccess.Resolve.pDstResource = resolve.m_dest->get_native<ID3D12Resource>();
					rtvs[i].EndingAccess.Resolve.PreserveResolveSource = resolve.m_keep_source;
					rtvs[i].EndingAccess.Resolve.pSubresourceParameters;
					rtvs[i].EndingAccess.Resolve.ResolveMode = D3D12_RESOLVE_MODE_MIN;
					rtvs[i].EndingAccess.Resolve.pSubresourceParameters;
					rtvs[i].EndingAccess.Resolve.SubresourceCount = 0u;
				}

				// store : preserve
				if (attachment.m_store == e_store_op::preserve)
				{
					rtvs[i].EndingAccess.PreserveLocal.AdditionalHeight = 0u;
					rtvs[i].EndingAccess.PreserveLocal.AdditionalWidth = 0u;
				}
			}

			D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* dsv = nullptr;
			if (args.m_depth_attachment.m_is_enabled)
			{
				static D3D12_RENDER_PASS_DEPTH_STENCIL_DESC dsv_desc{};
				dsv_desc.cpuDescriptor.ptr = (SIZE_T)args.m_depth_attachment.m_dsv_descriptor;
				dsv_desc.DepthBeginningAccess = translate(args.m_depth_attachment.m_depth_load);
				dsv_desc.StencilBeginningAccess = translate(args.m_depth_attachment.m_stencil_load);
				dsv_desc.DepthEndingAccess = translate(args.m_depth_attachment.m_depth_store);
				dsv_desc.StencilEndingAccess = translate(args.m_depth_attachment.m_stencil_store);
				dsv_desc.StencilBeginningAccess.Clear.ClearValue.DepthStencil.Stencil = args.m_depth_attachment.m_stencil_clear;
				dsv_desc.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth = args.m_depth_attachment.m_depth_clear;
				dsv_desc.DepthBeginningAccess.Clear.ClearValue.Format = DXGI_FORMAT_D32_FLOAT;
				dsv = &dsv_desc;
			}
			
			D3D12_RENDER_PASS_FLAGS flags = translate(args.m_flags);
			gfx_commandlist7->BeginRenderPass((uint32)rtvs.size(), rtvs.data(), dsv, flags);

			m_is_in_renderpass = true;
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
		if (is_in_renderpass() && res == S_OK && gfx_commandlist7 != nullptr)
		{
			gfx_commandlist7->EndRenderPass();
			m_is_in_renderpass = false;
		}
	}

	void dx12_commandlist::draw_instanced(const draw_instanced_args& args)
	{
		renderpass_check(e_command::draw_any);

		mpdx_graphics_commandlist->DrawInstanced(
			args.m_num_vertices_per_instance,
			args.m_num_instances,
			args.m_start_vertex,
			args.m_start_instance);
	}

	void dx12_commandlist::draw_indexed(const draw_indexed_args& args)
	{
		renderpass_check(e_command::draw_any);

		mpdx_graphics_commandlist->DrawIndexedInstanced(
			args.m_num_indexes_per_instance,
			args.m_num_instances,
			args.m_start_index,
			args.m_start_vertex,
			args.m_start_instance);
	}

	void dx12_commandlist::dispatch(const dispatch_args& args)
	{
		renderpass_check(e_command::dispatch);

		mpdx_graphics_commandlist->Dispatch(
			args.m_threadgroup_count.x,
			args.m_threadgroup_count.y,
			args.m_threadgroup_count.z);
	}

	void dx12_commandlist::set_constants(uint32 param_index, uint32 num_dwords, void* source_data, graphics::e_pipeline_type type)
	{
		renderpass_check(e_command::set_root_constants);

		switch (type)
		{
		case graphics::e_pipeline_type::compute:
			mpdx_graphics_commandlist->SetComputeRoot32BitConstants(
				param_index,
				num_dwords,
				source_data,
				0u);
			break;

		case graphics::e_pipeline_type::graphics:
			mpdx_graphics_commandlist->SetGraphicsRoot32BitConstants(
				param_index,
				num_dwords,
				source_data, 0u);
			break;
		}
	}

	void dx12_commandlist::set_indexbuffer(resource* index_buffer)
	{
		renderpass_check(e_command::set_indexbuffer);

		auto dxresource = index_buffer->get_native<ID3D12Resource>();

		D3D12_INDEX_BUFFER_VIEW ib_view{};
		ib_view.BufferLocation = dxresource->GetGPUVirtualAddress();
		ib_view.SizeInBytes = (uint32)index_buffer->get_bytesize();
		ib_view.Format = translate(index_buffer->get_format());

		mpdx_graphics_commandlist->IASetIndexBuffer(&ib_view);
	}

	void dx12_commandlist::set_vertexbuffer(resource* vertex_buffer)
	{
		renderpass_check(e_command::set_vertexbuffer);

		auto dxresource = vertex_buffer->get_native<ID3D12Resource>();

		D3D12_VERTEX_BUFFER_VIEW vb_view{};
		vb_view.BufferLocation = dxresource->GetGPUVirtualAddress();
		vb_view.SizeInBytes = (uint32)vertex_buffer->get_bytesize();
		vb_view.StrideInBytes = (uint32)vertex_buffer->get_bytestride();

		mpdx_graphics_commandlist->IASetVertexBuffers(0u, 1u, &vb_view);
	}

	void dx12_commandlist::clear_rtv(descriptor_handle rtv_cpu, const math::vectorf4& clear_value)
	{
		renderpass_check(e_command::clear_rtv);

		D3D12_CPU_DESCRIPTOR_HANDLE dx12_handle{};
		dx12_handle.ptr = (SIZE_T)rtv_cpu;
		mpdx_graphics_commandlist->ClearRenderTargetView(dx12_handle, clear_value.data(), 0u, nullptr);
	}

	void dx12_commandlist::clear_dsv(descriptor_handle dsv_cpu, float clear_depth, uint32 clear_stencil)
	{
		renderpass_check(e_command::clear_dsv);

		D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle{};
		cpu_handle.ptr = (SIZE_T)dsv_cpu;
		mpdx_graphics_commandlist->ClearDepthStencilView(cpu_handle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, clear_depth, clear_stencil, 0u, nullptr);
	}

	void dx12_commandlist::set_rtv(descriptor_handle rtv_cpu, descriptor_handle dsv_cpu)
	{
		renderpass_check(e_command::set_rtv);

		D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle{ .ptr = (SIZE_T)rtv_cpu  };
		D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle{ .ptr = (SIZE_T)dsv_cpu  };

		mpdx_graphics_commandlist->OMSetRenderTargets(1u, &rtv_handle, FALSE, dsv_cpu != nullptr ? &dsv_handle : nullptr);
	}

	void dx12_commandlist::set_srv(descriptor_handle srv_gpu, uint32 param_idx)
	{
		renderpass_check(e_command::set_srv);

		D3D12_GPU_DESCRIPTOR_HANDLE srv_gpu_handle{ .ptr = (SIZE_T)srv_gpu };
		mpdx_graphics_commandlist->SetGraphicsRootDescriptorTable(param_idx, srv_gpu_handle);
	}

	void dx12_commandlist::transition_resource(resource* resource, e_resource_state before, e_resource_state after)
	{
		renderpass_check(e_command::barrier_transition);

		auto dxresource = resource->get_native<ID3D12Resource>();
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(dxresource, translate(before), translate(after));
		mpdx_graphics_commandlist->ResourceBarrier(1u, &barrier);
	}

	void dx12_commandlist::buffer_barrier(resource* resource, e_resource_state before, e_resource_state after)
	{
		renderpass_check(e_command::barrier_any);

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
		renderpass_check(e_command::barrier_any);

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
		renderpass_check(e_command::barrier_any);

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
		renderpass_check(e_command::copy_resource);

		auto dxsource = source->get_native<ID3D12Resource>();
		auto dxdest = dest->get_native<ID3D12Resource>();

		mpdx_graphics_commandlist->CopyResource(dxdest, dxsource);
	}

	void dx12_commandlist::copy_texture(resource* src, resource* dest, 
		const copy_texture_args& args)
	{
		renderpass_check(e_command::copy_texture);

		CD3DX12_TEXTURE_COPY_LOCATION src_loc{ src->get_native<ID3D12Resource>() };
		src_loc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src_loc.PlacedFootprint.Offset = 0;
		src_loc.PlacedFootprint.Footprint.Width = dest->get_width();
		src_loc.PlacedFootprint.Footprint.Height = dest->get_height();
		src_loc.PlacedFootprint.Footprint.Depth = 1;
		src_loc.PlacedFootprint.Footprint.Format = translate(dest->get_format());
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
		renderpass_check(e_command::copy_buffer);

		mpdx_graphics_commandlist->CopyBufferRegion(
			dest->get_native<ID3D12Resource>(), args.m_dest_offset,
			src->get_native<ID3D12Resource>(), args.m_src_offset,
			bytesize);
	}

	void dx12_commandlist::set(descriptor_heap* heap)
	{
		renderpass_check(e_command::set_descriptor_heap);

		auto dxheap = heap->get_native<ID3D12DescriptorHeap>();
		mpdx_graphics_commandlist->SetDescriptorHeaps(1u, &dxheap);
	}

	void dx12_commandlist::set(const vector<descriptor_heap*>& heaps)
	{
		renderpass_check(e_command::set_descriptor_heap);

		vector<ID3D12DescriptorHeap*> native_heaps{};
		native_heaps.resize(heaps.size());
		for (uint64 i = 0u; i < heaps.size(); ++i)
		{
			native_heaps[i] = heaps[i]->get_native<ID3D12DescriptorHeap>();
		}

		mpdx_graphics_commandlist->SetDescriptorHeaps(static_cast<uint32>(native_heaps.size()), native_heaps.data());
	}

	void dx12_commandlist::set(const descriptor_range& gpu_range, uint32 param_idx)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle{};
		gpu_handle.ptr = (size_t)gpu_range.m_start;
		mpdx_graphics_commandlist->SetGraphicsRootDescriptorTable(param_idx, gpu_handle);
	}

	void dx12_commandlist::set(rootsignature* rootsig, const e_pipeline_type type)
	{
		auto dxrootsig = rootsig->get_native<ID3D12RootSignature>();
	
		switch (type)
		{
		case e_pipeline_type::graphics:
			mpdx_graphics_commandlist->SetGraphicsRootSignature(dxrootsig);
			break;

		case e_pipeline_type::compute:
			mpdx_graphics_commandlist->SetComputeRootSignature(dxrootsig);
			break;

		case e_pipeline_type::raytracing:
			break;
		}
		
	}

	void dx12_commandlist::set(detail::base_pipeline* pipeline)
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
		mpdx_graphics_commandlist->IASetPrimitiveTopology(translate(topo));
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

	bool dx12_commandlist::is_in_renderpass() const
	{
		return m_is_in_renderpass;
	}

	bool dx12_commandlist::is_renderpass_valid(e_command command) const
	{
		if (is_in_renderpass())
		{
			return is_allowed_in_renderpass(command);
		}

		return true;
	}
}