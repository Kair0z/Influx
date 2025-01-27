#pragma once

// dx12
#include "dx12_headers.h"

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
	private:
		ID3D12CommandList* mpdx_commandlist;
		ID3D12GraphicsCommandList* mpdx_graphics_commandlist;
		ID3D12CommandAllocator* mpdx_allocator;

		vector<D3D12_TEXTURE_BARRIER>		  m_texture_barriers;
		vector<D3D12_BUFFER_BARRIER>		  m_buffer_barriers;
		vector<D3D12_GLOBAL_BARRIER>		  m_global_barriers;

	private:
		dx12_commandlist(ID3D12GraphicsCommandList* commandlist, ID3D12CommandAllocator* allocator);
		virtual void release_impl(device*) override;
		friend class dx12_device;

		// starts a commandlist, using the device to allocate the memory internally
		virtual void start_impl(device*, pipeline* init_state = nullptr) override;

		virtual void renderpass_begin(const renderpass_args&) override;

		virtual void renderpass_end() override;

		virtual void draw_instanced(const draw_instanced_args&) override;

		virtual void draw_indexed(const draw_indexed_args&) override;

		virtual void set_constants(uint32 param_index, uint32 num_dwords, void* source_data) override;

		virtual void set_indexbuffer(resource* index_buffer) override;

		virtual void set_vertexbuffer(resource* vertex_buffer) override;

		virtual void clear_rtv(descriptor_handle rtv_cpu, const math::vectorf4& clear_value) override;

		virtual void clear_dsv(descriptor_handle dsv_cpu, float clear_depth, uint32 clear_stencil) override;

		virtual void set_rtv(descriptor_handle rtv_cpu, descriptor_handle dsv_cpu) override;

		virtual void set_srv(descriptor_handle srv_gpu, uint32 param_idx) override;

		virtual void transition_resource(resource* resource, e_resource_state before, e_resource_state after) override;

		// https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html
		virtual void buffer_barrier(resource* resource, e_resource_state before, e_resource_state after) override;

		virtual void texture_barrier(resource* resource, e_resource_state before, e_resource_state after) override;

		virtual void global_barrier(e_resource_state before, e_resource_state after) override;

		virtual void flush_barriers() override;

		virtual void copy_resource(resource* source, resource* dest) override;

		virtual void copy_texture(resource* src, resource* dest, const copy_texture_args& = {}) override;

		virtual void copy_buffer(resource* src, resource* dest, uint32 bytesize, const copy_buffer_args& = {}) override;

		virtual void set(descriptor_heap* heap) override;

		virtual void set(const vector<descriptor_heap*>& heap) override;

		virtual void set(const descriptor_range& gpu_range, uint32 param_idx) override;

		virtual void set(rootsignature* rootsig) override;

		virtual void set(pipeline* pipeline) override;

		virtual void set(const viewport& viewport) override;

		virtual void set(const rect& rect) override;

		virtual void set(e_primitive_topology topo) override;

		virtual void end() override;

		ID3D12CommandAllocator* obtain_allocator(dx12_device*);

		void free_allocator(dx12_device*);

		bool is_in_renderpass() const;

	private:
		bool m_is_in_renderpass = false;

		bool is_renderpass_valid(e_command command) const;

		// https://learn.microsoft.com/en-us/windows/win32/direct3d12/direct3d-12-render-passes
		// renderpasses prohibit any of these commands to be called.
		inline constexpr static bool is_allowed_in_renderpass(e_command command)
		{
			switch (command)
			{
				case e_command::atomic_copy_buffer: return false;
				case e_command::begin_renderpass  : return false;
				case e_command::clear_dsv		  : return false;
				case e_command::clear_rtv		  : return false;
				case e_command::clear_state		  : return false;
				case e_command::clear_uav		  : return false;
				case e_command::copy_buffer		  : return false;
				case e_command::copy_resource	  : return false;
				case e_command::copy_texture	  : return false;
				case e_command::copy_tiles		  : return false;
				case e_command::discard			  : return false;
				case e_command::dispatch		  : return false;
				case e_command::set_rtv			  : return false;
				case e_command::resolve_any		  : return false;
			}
			return true;
		}
	};
}