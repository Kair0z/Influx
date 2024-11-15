#pragma once

// influx::graphics
#include "influx_graphics/commandlist.h"

struct ID3D12CommandList;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;

namespace influx::graphics
{
	class pipeline;
	class render_target_view;
	class dx12_device;

	class dx12_commandlist final : public commandlist
	{
	public:
		dx12_commandlist(ID3D12GraphicsCommandList* commandlist, ID3D12CommandAllocator* allocator);

		// starts a commandlist, using the device to allocate the memory internally
		virtual void start_impl(device*, pipeline* init_state = nullptr) override;

		virtual void renderpass_begin(const renderpass_args&) override;

		virtual void renderpass_end() override;

		virtual void draw_instanced(const draw_instanced_args&) override;

		virtual void draw_indexed(const draw_indexed_args&) override;

		virtual void set_constants(uint32 param_index, uint32 num_dwords, void* source_data) override;

		virtual void set_indexbuffer(resource* index_buffer) override;

		virtual void set_vertexbuffer(resource* vertex_buffer) override;

		virtual void clear_rtv(render_target_view* view, const math::vectorf4& clear_value) override;

		virtual void clear_dsv(depth_stencil_view* view, float clear_depth, uint32 clear_stencil) override;

		virtual void transition_resource(resource* resource, e_resource_state before, e_resource_state after) override;

		virtual void copy_resource(resource* source, resource* dest) override;

		virtual void copy_texture(resource* src, resource* dest, const copy_texture_args& = {}) override;

		virtual void copy_buffer(resource* src, resource* dest, uint32 bytesize, const copy_buffer_args& = {}) override;

		virtual void set(descriptor_heap* heap) override;

		virtual void set(render_target_view* rtv, depth_stencil_view* dsv) override;

		virtual void set(shader_resource_view* srv, uint32 param_idx) override;

		virtual void set(const descriptor_range& gpu_range, uint32 param_idx) override;

		virtual void set(rootsignature* rootsig) override;

		virtual void set(pipeline* pipeline) override;

		virtual void set(const viewport& viewport) override;

		virtual void set(const rect& rect) override;

		virtual void set(e_primitive_topology topo) override;

		virtual void end() override;

		virtual void release() override;

		ID3D12CommandAllocator* obtain_allocator(dx12_device*);

		void free_allocator(dx12_device*);

	private:
		ID3D12CommandList* mpdx_commandlist;
		ID3D12GraphicsCommandList* mpdx_graphics_commandlist;
		ID3D12CommandAllocator* mpdx_allocator;
	};
}