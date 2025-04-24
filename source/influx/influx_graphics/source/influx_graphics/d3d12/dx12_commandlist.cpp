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
			return result<>::make_error("error: this command is not allowed inside a renderpass!"); \
		}\
		
	dx12_commandlist::dx12_commandlist(ID3D12GraphicsCommandList* commandlist, ID3D12CommandAllocator* allocator)
	{
		mp_native = mpdx_commandlist = mpdx_graphics_commandlist = commandlist;
		mpdx_allocator = allocator;
	}

	result<> dx12_commandlist::start_impl(device* device, detail::base_pipeline* init_state)
	{
		dx12_device* dxdevice = ((dx12_device*)device);

		if (mpdx_allocator != nullptr)
		{
			free_allocator(dxdevice);
		}

		mpdx_allocator = obtain_allocator(dxdevice);

		ID3D12PipelineState* dxpipeline = (init_state ? init_state->get_native<ID3D12PipelineState>() : nullptr);
		mpdx_graphics_commandlist->Reset(mpdx_allocator, dxpipeline);

		return {};
	}

	result<> dx12_commandlist::renderpass_begin(const renderpass_args& args)
	{
		if (!is_renderpass_valid(e_command::begin_renderpass))
		{
			return result<>::make_error("renderpass_begin is not allowed inside a renderpass!");
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
		return {};
	}

	result<> dx12_commandlist::renderpass_end()
	{
		ID3D12GraphicsCommandList7* gfx_commandlist7 = nullptr;
		HRESULT res = mpdx_graphics_commandlist->QueryInterface(&gfx_commandlist7);
		if (is_in_renderpass() && res == S_OK && gfx_commandlist7 != nullptr)
		{
			gfx_commandlist7->EndRenderPass();
			m_is_in_renderpass = false;
		}
		return {};
	}

	result<> dx12_commandlist::draw_instanced(const draw_instanced_args& args)
	{
		renderpass_check(e_command::draw_any);

		pre_draw();

		mpdx_graphics_commandlist->DrawInstanced(
			args.m_num_vertices_per_instance,
			args.m_num_instances,
			args.m_start_vertex,
			args.m_start_instance);
		return {};
	}

	result<> dx12_commandlist::draw_indexed(const draw_indexed_args& args)
	{
		renderpass_check(e_command::draw_any);

		pre_draw();

		mpdx_graphics_commandlist->DrawIndexedInstanced(
			args.m_num_indexes_per_instance,
			args.m_num_instances,
			args.m_start_index,
			args.m_start_vertex,
			args.m_start_instance);
		return {};
	}

	result<> dx12_commandlist::dispatch(const dispatch_args& args)
	{
		renderpass_check(e_command::dispatch);

		mpdx_graphics_commandlist->Dispatch(
			args.m_threadgroup_count.x,
			args.m_threadgroup_count.y,
			args.m_threadgroup_count.z);
		return {};
	}

	result<> dx12_commandlist::set_constants(uint32 param_index, uint32 num_dwords, void* source_data, graphics::e_pipeline_type type)
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

		case graphics::e_pipeline_type::mesh:
		case graphics::e_pipeline_type::graphics:
			mpdx_graphics_commandlist->SetGraphicsRoot32BitConstants(
				param_index,
				num_dwords,
				source_data, 0u);
			break;
		}
		return {};
	}

	result<> dx12_commandlist::set_indexbuffer(resource* index_buffer)
	{
		renderpass_check(e_command::set_indexbuffer);

		auto dxresource = index_buffer->get_native<ID3D12Resource>();

		D3D12_INDEX_BUFFER_VIEW ib_view{};
		ib_view.BufferLocation = dxresource->GetGPUVirtualAddress();
		ib_view.SizeInBytes = (uint32)index_buffer->get_bytesize();
		ib_view.Format = translate(index_buffer->get_format());

		mpdx_graphics_commandlist->IASetIndexBuffer(&ib_view);
		return {};
	}

	result<> dx12_commandlist::set_vertexbuffer(resource* vertex_buffer)
	{
		renderpass_check(e_command::set_vertexbuffer);

		auto dxresource = vertex_buffer->get_native<ID3D12Resource>();

		D3D12_VERTEX_BUFFER_VIEW vb_view{};
		vb_view.BufferLocation = dxresource->GetGPUVirtualAddress();
		vb_view.SizeInBytes = (uint32)vertex_buffer->get_bytesize();
		vb_view.StrideInBytes = (uint32)vertex_buffer->get_bytestride();

		mpdx_graphics_commandlist->IASetVertexBuffers(0u, 1u, &vb_view);
		return {};
	}

	result<> dx12_commandlist::clear_rtv(descriptor_handle rtv_cpu, const math::vectorf4& clear_value)
	{
		renderpass_check(e_command::clear_rtv);

		D3D12_CPU_DESCRIPTOR_HANDLE dx12_handle{};
		dx12_handle.ptr = (SIZE_T)rtv_cpu;
		mpdx_graphics_commandlist->ClearRenderTargetView(dx12_handle, clear_value.data(), 0u, nullptr);
		return {};
	}

	result<> dx12_commandlist::clear_dsv(descriptor_handle dsv_cpu, float clear_depth, uint32 clear_stencil)
	{
		renderpass_check(e_command::clear_dsv);

		D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle{};
		cpu_handle.ptr = (SIZE_T)dsv_cpu;
		mpdx_graphics_commandlist->ClearDepthStencilView(cpu_handle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, clear_depth, clear_stencil, 0u, nullptr);
		return {};
	}

	result<> dx12_commandlist::set_rtv(descriptor_handle rtv_cpu, descriptor_handle dsv_cpu)
	{
		renderpass_check(e_command::set_rtv);

		D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle{ .ptr = (SIZE_T)rtv_cpu  };
		D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle{ .ptr = (SIZE_T)dsv_cpu  };

		mpdx_graphics_commandlist->OMSetRenderTargets(1u, &rtv_handle, FALSE, dsv_cpu != nullptr ? &dsv_handle : nullptr);
		return {};
	}

	result<> dx12_commandlist::transition_resource(resource* resource, e_resource_state before, e_resource_state after)
	{
		renderpass_check(e_command::barrier_transition);

		auto dxresource = resource->get_native<ID3D12Resource>();
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(dxresource, translate(before), translate(after));
		mpdx_graphics_commandlist->ResourceBarrier(1u, &barrier);
		return {};
	}

	result<> dx12_commandlist::buffer_barrier(resource* resource, e_resource_state before, e_resource_state after)
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
		return {};
	}

	result<> dx12_commandlist::texture_barrier(resource* resource, e_resource_state before, e_resource_state after)
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
		return {};
	}

	result<> dx12_commandlist::global_barrier(e_resource_state before, e_resource_state after)
	{
		renderpass_check(e_command::barrier_any);

		D3D12_GLOBAL_BARRIER barrier{};
		barrier.SyncBefore = get_barrier_sync(before);
		barrier.SyncAfter = get_barrier_sync(after);
		barrier.AccessBefore = get_barrier_access(before);
		barrier.AccessAfter = get_barrier_access(after);
		m_global_barriers.push_back(barrier);
		return {};
	}

	result<> dx12_commandlist::flush_barriers()
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
		return {};
	}

	result<> dx12_commandlist::update_blas(blas_resources* blas, const blas_update_args& args)
	{
		ID3D12Resource* dxblas = blas->m_blas_buffer->get_native<ID3D12Resource>();
		ID3D12Resource* dxscratch = blas->m_scratch_buffer->get_native<ID3D12Resource>();

		if (blas->does_update_fit(args))
			return result<>::make_error("error: blas is not big enough for this update!");

		// map new data into buffer
		const vector<blas_update_args::vertex>& vertices = args.m_vertices;
		const vector<blas_update_args::index>& indices = args.m_indices;
		resource* index_buffer = blas->m_index_buffer;
		resource* vertex_buffer = blas->m_vertex_buffer;
		index_buffer->map([&args, &indices](void* dest)
		{
			memcpy(dest, indices.data(), indices.size() * sizeof(blas_update_args::index));
		});
		vertex_buffer->map([&args, &vertices](void* dest)
		{
			memcpy(dest, vertices.data(), vertices.size() * sizeof(blas_update_args::vertex));
		});

		// describe geometry
		ID3D12Resource* dxvertexbuffer = vertex_buffer->get_native<ID3D12Resource>();
		ID3D12Resource* dxindexbuffer = index_buffer->get_native<ID3D12Resource>();
		D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc =
		{
			.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES,
			.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE,
			.Triangles =
			{
				.Transform3x4 = 0,
				.IndexFormat = DXGI_FORMAT_R16_UINT,
				.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT,
				.IndexCount = index_buffer->get_num_elements(),
				.VertexCount = vertex_buffer->get_num_elements(),
				.IndexBuffer = dxindexbuffer->GetGPUVirtualAddress(),
				.VertexBuffer =
				{
					.StartAddress = dxvertexbuffer->GetGPUVirtualAddress(),
					.StrideInBytes = vertex_buffer->get_bytestride()
				}
			}
		};

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC desc{};
		desc.DestAccelerationStructureData = dxblas->GetGPUVirtualAddress();
		desc.ScratchAccelerationStructureData = dxscratch->GetGPUVirtualAddress();
		desc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		desc.Inputs.pGeometryDescs = &geometryDesc;
		desc.Inputs.NumDescs = 1u;
		desc.Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
		desc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;

		ID3D12GraphicsCommandList4* dxcommandlist4 = nullptr;
		HRESULT
		hres = mpdx_graphics_commandlist->QueryInterface<ID3D12GraphicsCommandList4>(&dxcommandlist4);
		if (hres == S_OK && dxcommandlist4 != nullptr)
		{
			dxcommandlist4->BuildRaytracingAccelerationStructure(&desc, 0u, nullptr);
		}
		else
		{
			return result<>::make_error("error: QueryInterface<ID3D12GraphicsCommandList4> failed!");
		}

		return {};
	}

	result<> dx12_commandlist::update_tlas(tlas_resources* tlas, const tlas_update_args& args)
	{
		if (args.m_blas == nullptr)
			return result<>::make_error("error: invalid blas!");
		if (tlas == nullptr || tlas->m_instances_buffer == nullptr || tlas->m_scratch_buffer == nullptr || tlas->m_tlas_buffer == nullptr)
			return result<>::make_error("error: invalid tlas!");
		if (tlas->does_update_fit(args))
			return result<>::make_error("error: tlas is not big enough for this update!");

		ID3D12Resource* dxblas = args.m_blas->m_blas_buffer->get_native<ID3D12Resource>();
		D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
		instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;
		instanceDesc.InstanceMask = 1;
		instanceDesc.AccelerationStructure = dxblas->GetGPUVirtualAddress();

		// map new data into buffer
		tlas->m_instances_buffer->map([&instanceDesc](void* dest)
		{
			memcpy(dest, &instanceDesc, sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
		});

		ID3D12Resource* dxtlas = tlas->m_tlas_buffer->get_native<ID3D12Resource>();
		ID3D12Resource* dxscratch = tlas->m_scratch_buffer->get_native<ID3D12Resource>();
		ID3D12Resource* dxinstances = tlas->m_instances_buffer->get_native<ID3D12Resource>();

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC desc{};
		desc.DestAccelerationStructureData = dxtlas->GetGPUVirtualAddress();
		desc.ScratchAccelerationStructureData = dxscratch->GetGPUVirtualAddress();
		desc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		desc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		desc.Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
		desc.Inputs.InstanceDescs = dxinstances->GetGPUVirtualAddress();
		desc.Inputs.NumDescs = 1u;

		ID3D12GraphicsCommandList4* dxcommandlist4 = nullptr;
		HRESULT
			hres = mpdx_graphics_commandlist->QueryInterface<ID3D12GraphicsCommandList4>(&dxcommandlist4);
		if (hres == S_OK && dxcommandlist4 != nullptr)
		{
			dxcommandlist4->BuildRaytracingAccelerationStructure(&desc, 0u, nullptr);
		}
		else
		{
			return result<>::make_error("error: QueryInterface<ID3D12GraphicsCommandList4> failed!");
		}

		return {};
	}

	result<> dx12_commandlist::copy_resource(resource* source, resource* dest)
	{
		renderpass_check(e_command::copy_resource);
		influx_assert(source->get_width() == dest->get_width());
		influx_assert(source->get_height() == dest->get_height());

		auto dxsource = source->get_native<ID3D12Resource>();
		auto dxdest = dest->get_native<ID3D12Resource>();

		mpdx_graphics_commandlist->CopyResource(dxdest, dxsource);
		return {};
	}

	result<> dx12_commandlist::copy_texture(resource* src, resource* dest,
		const copy_texture_args& args)
	{
		// https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist-copytextureregion
		renderpass_check(e_command::copy_texture);

		const bool source_is_texture = src->get_type() != resource::e_type::buffer;
		const bool dest_is_texture = dest->get_type() != resource::e_type::buffer;
		influx_assert(source_is_texture || dest_is_texture);

		const uint32 num_subresources = math::maximum(1u, dest->get_arraysize());
		
		vector<resource::footprint> src_footprints = source_is_texture ? src->get_footprints() : vector<resource::footprint>{};
		vector<resource::footprint> dest_footprints = dest_is_texture ? dest->get_footprints() : vector<resource::footprint>{};
		if (src_footprints.empty()) src_footprints = dest_footprints;
		if (dest_footprints.empty()) dest_footprints = src_footprints;

		D3D12_BOX src_box{};
		src_box.left = 0u;
		src_box.top = 0u;
		src_box.front = 0u;
		src_box.right = dest->get_width();
		src_box.bottom = dest->get_height();
		src_box.back = dest->get_depth();

		for (uint32 i = 0u; i < num_subresources; ++i)
		{
			D3D12_TEXTURE_COPY_LOCATION src_location{};
			src_location.PlacedFootprint.Footprint.Depth = src_footprints[i].m_depth;
			src_location.PlacedFootprint.Footprint.Format = translate(src_footprints[i].m_format);
			src_location.PlacedFootprint.Footprint.Height = src_footprints[i].m_height;
			src_location.PlacedFootprint.Footprint.RowPitch	= static_cast<uint32>(src_footprints[i].m_row_bytesize);
			src_location.PlacedFootprint.Footprint.Width = src_footprints[i].m_width;
			src_location.PlacedFootprint.Offset = src_footprints[i].m_offset;
			src_location.SubresourceIndex = i;
			src_location.Type = source_is_texture ? D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX : D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			src_location.pResource = src->get_native<ID3D12Resource>();

			D3D12_TEXTURE_COPY_LOCATION dst_location{};
			dst_location.PlacedFootprint.Footprint.Depth = dest_footprints[i].m_depth;
			dst_location.PlacedFootprint.Footprint.Format = translate(dest_footprints[i].m_format);
			dst_location.PlacedFootprint.Footprint.Height = dest_footprints[i].m_height;
			dst_location.PlacedFootprint.Footprint.RowPitch = static_cast<uint32>(dest_footprints[i].m_row_bytesize);
			dst_location.PlacedFootprint.Footprint.Width = dest_footprints[i].m_width;
			dst_location.PlacedFootprint.Offset = dest_footprints[i].m_offset;
			dst_location.SubresourceIndex = i;
			dst_location.Type = dest_is_texture ? D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX : D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			dst_location.pResource = dest->get_native<ID3D12Resource>();

			mpdx_graphics_commandlist->CopyTextureRegion(&dst_location, 0u, 0u, 0u, &src_location, &src_box);
		}

		return {};
	}

	result<> dx12_commandlist::copy_buffer(resource* src, resource* dest, uint32 bytesize, const copy_buffer_args& args)
	{
		renderpass_check(e_command::copy_buffer);

		mpdx_graphics_commandlist->CopyBufferRegion(
			dest->get_native<ID3D12Resource>(), args.m_dest_offset,
			src->get_native<ID3D12Resource>(), args.m_src_offset,
			bytesize);

		return {};
	}

	result<> dx12_commandlist::set(descriptor_heap* heap)
	{
		renderpass_check(e_command::set_descriptor_heap);

		auto dxheap = heap->get_native<ID3D12DescriptorHeap>();
		mpdx_graphics_commandlist->SetDescriptorHeaps(1u, &dxheap);

		return {};
	}

	result<> dx12_commandlist::set(const vector<descriptor_heap*>& heaps)
	{
		renderpass_check(e_command::set_descriptor_heap);

		vector<ID3D12DescriptorHeap*> native_heaps{};
		native_heaps.resize(heaps.size());
		for (uint64 i = 0u; i < heaps.size(); ++i)
		{
			native_heaps[i] = heaps[i]->get_native<ID3D12DescriptorHeap>();
		}

		mpdx_graphics_commandlist->SetDescriptorHeaps(static_cast<uint32>(native_heaps.size()), native_heaps.data());
		return {};
	}

	result<> dx12_commandlist::set_srv(resource* root_resource, uint32 param_idx, const e_pipeline_type type)
	{
		renderpass_check(e_command::set_srv);

		ID3D12Resource* dxresource = root_resource->get_native<ID3D12Resource>();
		switch (type)
		{
		default:
		case e_pipeline_type::graphics:
		{
			mpdx_graphics_commandlist->SetGraphicsRootShaderResourceView(param_idx, dxresource->GetGPUVirtualAddress());
		}break;

		case e_pipeline_type::compute:
		case e_pipeline_type::raytracing:
		{
			mpdx_graphics_commandlist->SetComputeRootShaderResourceView(param_idx, dxresource->GetGPUVirtualAddress());
		}break;
		}

		return {};
	}

	result<> dx12_commandlist::set_uav(resource* root_resource, uint32 param_idx, const e_pipeline_type type)
	{
		renderpass_check(e_command::set_uav);

		ID3D12Resource* dxresource = root_resource->get_native<ID3D12Resource>();
		switch (type)
		{
		default:
		case e_pipeline_type::graphics:
		{
			mpdx_graphics_commandlist->SetGraphicsRootUnorderedAccessView(param_idx, dxresource->GetGPUVirtualAddress());
		}break;

		case e_pipeline_type::compute:
		case e_pipeline_type::raytracing:
		{
			mpdx_graphics_commandlist->SetComputeRootUnorderedAccessView(param_idx, dxresource->GetGPUVirtualAddress());
		}break;
		}

		return {};
	}

	result<> dx12_commandlist::set(const descriptor_range& gpu_range, uint32 param_idx, const e_pipeline_type type)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle{};
		gpu_handle.ptr = (size_t)gpu_range.m_start;

		switch (type)
		{
		default:
		case e_pipeline_type::graphics:
		{
			mpdx_graphics_commandlist->SetGraphicsRootDescriptorTable(param_idx, gpu_handle);
		}break;

		case e_pipeline_type::compute:
		case e_pipeline_type::raytracing:
		{
			mpdx_graphics_commandlist->SetComputeRootDescriptorTable(param_idx, gpu_handle);
		}break;
		}
		
		return {};
	}

	result<> dx12_commandlist::set(rootsignature* rootsig, const e_pipeline_type type)
	{
		auto dxrootsig = rootsig->get_native<ID3D12RootSignature>();
		
		switch (type)
		{
		case e_pipeline_type::graphics:
			mpdx_graphics_commandlist->SetGraphicsRootSignature(dxrootsig);
			break;

		case e_pipeline_type::compute:
		case e_pipeline_type::raytracing:
			mpdx_graphics_commandlist->SetComputeRootSignature(dxrootsig);
			break;

		default:
		case e_pipeline_type::mesh:
			break;
		}

		return {};
	}

	result<> dx12_commandlist::set(detail::base_pipeline* pipeline)
	{
		switch (pipeline->get_type())
		{
		case e_pipeline_type::raytracing:
		{
			ID3D12GraphicsCommandList4* dxcommandlist4 = nullptr;
			HRESULT hres = mpdx_graphics_commandlist->QueryInterface<ID3D12GraphicsCommandList4>(&dxcommandlist4);
			if (hres == S_OK)
			{
				auto dxstateobject = pipeline->get_native<ID3D12StateObject>();
				dxcommandlist4->SetPipelineState1(dxstateobject);
			}
			else return result<>::make_error("error: QueryInterface<ID3D12GraphicsCommandList4> failed!");
		}break;

		default:
		{
			auto dxpipeline = pipeline->get_native<ID3D12PipelineState>();
			mpdx_graphics_commandlist->SetPipelineState(dxpipeline);
		}break;
		}
		
		return {};
	}

	result<> dx12_commandlist::set(const viewport& viewport)
	{
		commandlist::set(viewport);

		D3D12_VIEWPORT dxviewport{};
		dxviewport.TopLeftX = viewport.m_left;
		dxviewport.TopLeftY = viewport.m_top;
		dxviewport.Width = viewport.m_width;
		dxviewport.Height = viewport.m_height;
		dxviewport.MinDepth = viewport.m_depth_min;
		dxviewport.MaxDepth = viewport.m_depth_max;
		mpdx_graphics_commandlist->RSSetViewports(1u, &dxviewport);
		return {};
	}

	result<> dx12_commandlist::set(const rect& rect)
	{
		commandlist::set(rect);

		D3D12_RECT dxrect{};
		dxrect.left = rect.m_left;
		dxrect.top = rect.m_top;
		dxrect.right = rect.m_right;
		dxrect.bottom = rect.m_bottom;
		mpdx_graphics_commandlist->RSSetScissorRects(1u, &dxrect);
		return {};
	}

	result<> dx12_commandlist::set(e_primitive_topology topo)
	{
		mpdx_graphics_commandlist->IASetPrimitiveTopology(translate(topo));
		return {};
	}

	result<> dx12_commandlist::end()
	{
		mpdx_graphics_commandlist->Close();
		return {};
	}

	void dx12_commandlist::release_impl(device*)
	{
		mpdx_graphics_commandlist->Release();
	}

	result<> dx12_commandlist::dispatch_mesh(uint32 groupcount_x, uint32 groupcount_y, uint32 groupcount_z)
	{
		renderpass_check(e_command::dispatch_mesh);

		ID3D12GraphicsCommandList6* dxcommandlist6 = nullptr;
		HRESULT res = mpdx_graphics_commandlist->QueryInterface<ID3D12GraphicsCommandList6>(&dxcommandlist6);
		if (res == S_OK)
		{
			const bool is_viewport_valid = m_viewport.m_width > 0.0f && m_viewport.m_height > 0.0f;
			const bool is_rect_valid = m_scissor_rect.m_right > 0u && m_scissor_rect.m_bottom > 0u;

			dxcommandlist6->DispatchMesh(
				groupcount_x,
				groupcount_y,
				groupcount_z
			);

			return {};
		}

		return result<>::make_error("error: QueryInterface<ID3D12GraphicsCommandList6> failed!");
	}

	result<> dx12_commandlist::dispatch_rays(raytracing_pipeline* pipeline, uint32 width, uint32 height, uint32 depth)
	{
		renderpass_check(e_command::dispatch_rays);

		if (pipeline == nullptr)
			return result<>::make_error("error: invalid pipeline!");

		ID3D12GraphicsCommandList4* dxcommandlist4 = nullptr;
		HRESULT hres = mpdx_graphics_commandlist->QueryInterface<ID3D12GraphicsCommandList4>(&dxcommandlist4);
		if (hres == S_OK)
		{
			dx12_pipeline<e_pipeline_type::raytracing>* dx12_raypipeline = (dx12_pipeline<e_pipeline_type::raytracing>*)pipeline;
			ID3D12StateObject* dxstateobject = pipeline->get_native<ID3D12StateObject>();

			ID3D12Resource* HitGroupTableResource = dx12_raypipeline->m_hitgroup_shadertable.mpdx_resource;
			ID3D12Resource* MissTableResource = dx12_raypipeline->m_miss_shadertable.mpdx_resource;
			ID3D12Resource* RayGenTableResource = dx12_raypipeline->m_raygen_shadertable.mpdx_resource;

			D3D12_DISPATCH_RAYS_DESC desc{};
			desc.HitGroupTable.StartAddress = HitGroupTableResource->GetGPUVirtualAddress();
			desc.HitGroupTable.SizeInBytes = HitGroupTableResource->GetDesc().Width;
			desc.HitGroupTable.StrideInBytes = desc.HitGroupTable.SizeInBytes;
			desc.MissShaderTable.StartAddress = MissTableResource->GetGPUVirtualAddress();
			desc.MissShaderTable.SizeInBytes = MissTableResource->GetDesc().Width;
			desc.MissShaderTable.StrideInBytes = desc.MissShaderTable.SizeInBytes;
			desc.RayGenerationShaderRecord.StartAddress = RayGenTableResource->GetGPUVirtualAddress();
			desc.RayGenerationShaderRecord.SizeInBytes = RayGenTableResource->GetDesc().Width;
			desc.CallableShaderTable.SizeInBytes = 0u;
			desc.CallableShaderTable.StartAddress = 0u;
			desc.CallableShaderTable.StrideInBytes = 0u;
			desc.Width = width;
			desc.Height = height;
			desc.Depth = depth;

			dxcommandlist4->DispatchRays(&desc);
			return {};
		}

		return result<>::make_error("error: QueryInterface<ID3D12GraphicsCommandList4> failed!");
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