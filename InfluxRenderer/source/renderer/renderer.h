#pragma once

#include "influx_renderer.h"

#include "core/singleton/singleton.h"
#include "Core/Container/Vector.h"
#include "Core/Container/RingBuffer.h"
#include "Core/Container/List.h"
#include "Core/Container/Queue.h"
#include "Core/Geometry/Rect.h"
#include "Core/Time.h"
#include "Core/Math/Vector.h"
#include "Core/Container/Map.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include "foreign/d3dx12.h"

struct IDXGIFactory1;
struct IDXGIAdapter1;
struct ID3D12Device;
struct ID3D12CommandQueue;
struct ID3D12DescriptorHeap;
struct IDXGISwapChain4;
struct ID3D12CommandAllocator;
struct ID3D12Resource;
struct ID3D12Fence;
struct ID3D12GraphicsCommandList;
struct D3D12_CPU_DESCRIPTOR_HANDLE;
struct ID3D12RootSignature;
struct ID3D12PipelineState;
struct IDXGIFactory4;

namespace influx::renderer
{
	class renderer_state final
		: public singleton<renderer_state>
	{
	public:
		void initialize(const init_args& args);
		void initialize_imgui(platform::window_handle window);

		void render_to_window(
			const scene_proxy* scene_proxy,
			platform::window_handle window,
			const imgui_proxy* imgui_proxy,
			const render_args& render_args,
			const present_args& present);

		vector<frame_stats> get_frame_stats(const uint32 over_num_frames);
		bool is_initialized() const;
		bool is_initialized_imgui() const;
		void cleanup();

		void load(const string& title, const mesh_data& data);
		void load(const string& title, const texture_data& data);
		void load(const string& title, const material_data& data);

		const mesh_data* find_mesh_data(const string& title) const;
		vector<const mesh_data*> get_all_mesh_datas() const;

		void* get_backend_device() const;
		void* get_backend_texture_gpu_handle(const string& title) const;

		using frame_id = uint64;
		struct per_frame_context final
		{
			frame_id m_frame = 0u;
			ID3D12CommandAllocator* mpdx_commandAllocator = nullptr;
			ID3D12GraphicsCommandList* mpdx_commandList = nullptr;
			ID3D12Resource* mpdx_backbuffer = nullptr;
			D3D12_CPU_DESCRIPTOR_HANDLE* mpdx_rtv_handle;
			platform::event_handle m_complete_event = NULL;
			frame_stats m_stats{};

			time::point m_timepoint_created = time::get_now();
		};

	private:
		IDXGIFactory4* mpdx_factory = nullptr;
		ID3D12Device* mpdx_device = nullptr;
		ID3D12CommandQueue* mpdx_commandQueue = nullptr;
		IDXGISwapChain4* mpdx_swapchain = nullptr;
		ID3D12DescriptorHeap* mpdx_rtv_heap = nullptr;
		ID3D12DescriptorHeap* mpdx_srvheap = nullptr;
		ID3D12DescriptorHeap* mpdx_srvheap_imgui = nullptr;
		ID3D12RootSignature* mpdx_rootsignature = nullptr;
		ID3D12PipelineState* mpdx_pipeline = nullptr;
		CD3DX12_RECT m_rect{};
		CD3DX12_VIEWPORT m_viewport{};
		float m_aspect_ratio{};

		vector<ID3D12CommandAllocator*> mpdx_commandAllocators{};
		vector<ID3D12GraphicsCommandList*> mpdx_commandLists{};
		ID3D12Fence* mpdx_fence = nullptr;
		ID3D12Fence* mpdx_copy_fence = nullptr;
		uint64 m_copy_fence_value = 0u;
		vector<ID3D12Resource*> mpdx_backbufferResources{};
		vector<D3D12_CPU_DESCRIPTOR_HANDLE> mpdx_backbuffer_rtvs{};
		uint32 m_swapchain_buffer_idx = 0u;
		uint64 m_rtvDescriptorSize = 0u;
		uint64 m_srvDescriptorSize = 0u;
		frame_id m_frame = 0u;

		struct swapchain_state final
		{
			math::rectu m_window_rect{};
		};
		swapchain_state m_previous_swapchain_state{};
		bool is_swapchain_dirty(const swapchain_state& new_swapchain) const;

		struct view_constant_buffer final
		{
			math::matrix4x4f m_wvp{};
		};
		view_constant_buffer m_view_constant_buffer{};

		struct instance_data final
		{
			math::matrix4x4f m_transform{};
			math::vectorf4 m_colour{};
		};
		umap<string, vector<instance_data>> m_instance_map{};

		struct mesh_data_entry final
		{
			mesh_data m_data{};
			ID3D12Resource* mp_vertexbuffer = nullptr;
			ID3D12Resource* mp_indexbuffer = nullptr;
			ID3D12Resource* mp_instancebuffer = nullptr;
			D3D12_VERTEX_BUFFER_VIEW mdx_vertexbuffer_view{};
			D3D12_VERTEX_BUFFER_VIEW mdx_instancebuffer_view{};
			D3D12_INDEX_BUFFER_VIEW mdx_indexbuffer_view{};
			uint32 m_vertexbuffer_size = 0u;
			uint32 m_indexbuffer_size = 0u;
			uint32 m_instancebuffer_size = 0u;

			uint32 m_num_instances_this_frame = 0u;
		};
		struct texture_data_entry final
		{
			texture_data m_data{};
			ID3D12Resource* mp_resource = nullptr;
			uint32 m_resource_size = 0u;
			D3D12_CPU_DESCRIPTOR_HANDLE m_srv_handle_cpu;
			D3D12_GPU_DESCRIPTOR_HANDLE m_srv_handle_gpu;
		};
		struct material_data_entry final
		{
			material_data m_data{};
			ID3D12Resource* mp_resource = nullptr;
			D3D12_CPU_DESCRIPTOR_HANDLE m_cbv_handle_cpu;
			D3D12_GPU_DESCRIPTOR_HANDLE m_cbv_handle_gpu;
		};
		umap<string, mesh_data_entry> m_meshdata_map{};
		umap<string, texture_data_entry> m_texturedata_map{};
		umap<string, material_data_entry> m_materialdata_map{};

		// recreates the swapchain resources when it's dirty
		void recreate_swapchain_from_window(const e_buffering& buffering, platform::window_handle handle);
		void update_view_constant_buffer(const scene_proxy* proxy);
		void update_instance_buffers(const scene_proxy* proxy);
		per_frame_context acquire_next_frame(); // stalls if num_frames_in_flight > k_max_frames_in_flight
		void on_frame_finished(const per_frame_context& ctx);
		void submit_to_queue(const per_frame_context& ctx);
		bool get_swapchain_buffer_and_rtv(const frame_id for_frame, ID3D12Resource*& out_buffer, D3D12_CPU_DESCRIPTOR_HANDLE*& out_rtv);
		void transition_resource(ID3D12GraphicsCommandList* cmdlist, ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

		template <class _t>
		static void safe_release(_t*& ptr)
		{
			if (ptr != nullptr)
			{
				ptr->Release();
				ptr = nullptr;
			}
		}

	private:
		bool m_is_initialized = false;
		bool m_is_initialized_imgui = false;
		queue<per_frame_context> m_frames_in_flight{};
		ringbuffer<frame_stats, k_max_stat_frames> m_frame_stats{};
	};
}
