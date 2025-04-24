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
	class render_target_view;
	class dx12_device;

	class dx12_commandlist final : public commandlist
	{
	private:
		/* dx objects */
		ID3D12CommandList* mpdx_commandlist;
		ID3D12GraphicsCommandList* mpdx_graphics_commandlist;
		ID3D12CommandAllocator* mpdx_allocator;

		/* cache barriers to batch them together */
		vector<D3D12_TEXTURE_BARRIER>		  m_texture_barriers;
		vector<D3D12_BUFFER_BARRIER>		  m_buffer_barriers;
		vector<D3D12_GLOBAL_BARRIER>		  m_global_barriers;

	private:
		dx12_commandlist(ID3D12GraphicsCommandList* commandlist, ID3D12CommandAllocator* allocator);
		virtual void release_impl(device*) override;
		friend class dx12_device;

		// starts a commandlist, using the device to allocate the memory internally
		virtual result<> start_impl(device* device, detail::base_pipeline* init_state = nullptr) override;

		/* commands */
		virtual result<> renderpass_begin(const renderpass_args&) override;

		virtual result<> renderpass_end() override;

		/* draw indexed instanced */
		virtual result<> draw_instanced(const draw_instanced_args&) override;

		/* draw indexed */
		virtual result<> draw_indexed(const draw_indexed_args&) override;

		/* dispatch compute */
		virtual result<> dispatch(const dispatch_args&) override;

		/* set root constants at root param index 'x' (must match root signature param idx) */
		virtual result<> set_constants(uint32 param_index, uint32 num_dwords, void* source_data, graphics::e_pipeline_type type = e_pipeline_type::graphics) override;

		/* Input Assembler */
		virtual result<> set_indexbuffer(resource* index_buffer) override;
		virtual result<> set_vertexbuffer(resource* vertex_buffer) override;

		/* */
		virtual result<> clear_rtv(descriptor_handle rtv_cpu, const math::vectorf4& clear_value) override;

		/* */
		virtual result<> clear_dsv(descriptor_handle dsv_cpu, float clear_depth, uint32 clear_stencil) override;

		/* Output Merger - dsv_cpu is optional*/
		virtual result<> set_rtv(descriptor_handle rtv_cpu, descriptor_handle dsv_cpu) override;

		virtual result<> transition_resource(resource* resource, e_resource_state before, e_resource_state after) override;

		// https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html
		virtual result<> buffer_barrier(resource* resource, e_resource_state before, e_resource_state after) override;

		virtual result<> texture_barrier(resource* resource, e_resource_state before, e_resource_state after) override;

		virtual result<> global_barrier(e_resource_state before, e_resource_state after) override;

		virtual result<> flush_barriers() override;

		virtual result<> update_blas(blas_resources* blas, const blas_update_args& args) override;

		virtual result<> update_tlas(tlas_resources* tlas, const tlas_update_args& args) override;

		virtual result<> copy_resource(resource* source, resource* dest) override;

		virtual result<> copy_texture(resource* src, resource* dest, const copy_texture_args& = {}) override;

		virtual result<> copy_buffer(resource* src, resource* dest, uint32 bytesize, const copy_buffer_args& = {}) override;

		virtual result<> set(descriptor_heap* heap) override;

		virtual result<> set(const vector<descriptor_heap*>& heap) override;

		virtual result<> set_srv(resource* root_resource, uint32 param_idx, const e_pipeline_type type) override;

		virtual result<> set_uav(resource* root_resource, uint32 param_idx, const e_pipeline_type type) override;

		virtual result<> set(const descriptor_range& gpu_range, uint32 param_idx, const e_pipeline_type type = e_pipeline_type::graphics) override;

		virtual result<> set(rootsignature* rootsig, const e_pipeline_type type) override;

		virtual result<> set(detail::base_pipeline* pipeline) override;

		virtual result<> set(const viewport& viewport) override;

		virtual result<> set(const rect& rect) override;

		virtual result<> set(e_primitive_topology topo) override;

		virtual result<> end() override;

		/* mesh shaders */
		virtual result<> dispatch_mesh(uint32 groupcount_x, uint32 groupcount_y, uint32 groupcount_z) override;

		/* raytracing */
		virtual result<> dispatch_rays(raytracing_pipeline* pipeline, uint32 width, uint32 height, uint32 depth = 1) override;

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