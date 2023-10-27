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
		void render_to_window(const scene_proxy* scene_proxy, const render_args& render_args, platform::window_handle window, const present_args& present);
		vector<frame_stats> get_frame_stats(const uint32 over_num_frames);
		bool is_initialized() const;
		void cleanup();

		void load(const string& title, const mesh_data& data);
		void load(const string& title, const texture_data& data);

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
		ID3D12RootSignature* mpdx_rootsignature = nullptr;
		ID3D12PipelineState* mpdx_pipeline = nullptr;
		ID3D12Resource* mpdx_scene_vertexbuffer = nullptr;
		ID3D12Resource* mpdx_scene_indexbuffer = nullptr;
		D3D12_VERTEX_BUFFER_VIEW mdx_vertexbuffer_view{};
		D3D12_INDEX_BUFFER_VIEW mdx_indexbuffer_view{};

		vector<ID3D12CommandAllocator*> mpdx_commandAllocators{};
		vector<ID3D12GraphicsCommandList*> mpdx_commandLists{};
		ID3D12Fence* mpdx_fence = nullptr;
		vector<ID3D12Resource*> mpdx_backbufferResources{};
		vector<D3D12_CPU_DESCRIPTOR_HANDLE> mpdx_backbuffer_rtvs{};
		uint32 m_swapchain_buffer_idx = 0u;
		uint32 m_rtvDescriptorSize = 0u;
		uint32 m_srvDescriptorSize = 0u;

		umap<string, mesh_data> m_meshdata_map{};
		umap<string, texture_data> m_texturedata_map{};

		frame_id m_frame = 0u;

		// recreates the swapchain resources when it's dirty
		void recreate_swapchain_from_window(const e_buffering& buffering, platform::window_handle handle);
		void update_scene_buffers(const scene_proxy* proxy);

		// stalls if num_frames_in_flight > k_max_frames_in_flight
		per_frame_context acquire_next_frame();

		void on_frame_finished(const per_frame_context& ctx);

		// submit frame to queue
		void submit_to_queue(const per_frame_context& ctx);

		bool get_swapchain_buffer_and_rtv(const frame_id for_frame, ID3D12Resource*& out_buffer, D3D12_CPU_DESCRIPTOR_HANDLE*& out_rtv);

		struct swapchain_state final
		{
			math::rectu m_window_rect{};
		};
		swapchain_state m_previous_swapchain_state{};
		bool is_swapchain_dirty(const swapchain_state& new_swapchain) const;

		struct scene_geometry_info final
		{
			uint32 m_vertexbuffersize	= 0u;
			uint32 m_indexbuffersize	= 0u;
		};
		scene_geometry_info m_previous_scene_geometry{};

		template <class _t>
		void safe_release(_t*& ptr)
		{
			if (ptr != nullptr)
			{
				ptr->Release();
				ptr = nullptr;
			}
		}
	private:
		bool m_is_initialized = false;
		queue<per_frame_context> m_frames_in_flight{};
		ringbuffer<frame_stats, k_max_stat_frames> m_frame_stats{};
	};
}
