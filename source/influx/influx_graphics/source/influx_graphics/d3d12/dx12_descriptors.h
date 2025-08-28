#pragma once 

// influx::graphics
#include "influx_graphics/descriptors.h"
#include "influx_graphics/d3d12/dx12_base.h"
#include "core/container/list.h"

struct ID3D12DescriptorHeap;

#include "dx12_headers.h"

namespace influx::graphics
{
	class dx12_descriptor_heap final : public descriptor_heap
	{
		ID3D12DescriptorHeap* mpdx_heap;
		uint64 m_descriptor_stride;

		struct entry final
		{
			descriptor_id m_index = 0u;
			bool m_is_allocated = false;
		};

		vector<entry> m_freelist = {};
		friend class dx12_device;

	private:
		dx12_descriptor_heap(const descriptor_heap::create_args& args, 
			ID3D12DescriptorHeap* dxheap, uint64 descriptor_stride);

		/* allocate descriptors */
		virtual result<descriptor_id> allocate() override;

		virtual result<> free(descriptor_id handle) override;
		virtual result<> free_all() override;

		/* get the handles */
		virtual result<descriptor_handle> get_cpu(descriptor_id handle) const override;
		virtual result<descriptor_handle> get_gpu(descriptor_id handle) const override;
		virtual result<descriptor_id> get_id(descriptor_handle handle) const override;

	private:
		void clear();
		virtual void release_impl(device*) override;

		result<uint32> gpu_handle_to_index(descriptor_handle handle) const;
		result<uint32> cpu_handle_to_index(descriptor_handle handle) const;
		result<descriptor_handle> index_to_gpu_handle(uint32 index) const;
		result<descriptor_handle> index_to_cpu_handle(uint32 index) const;
	};

	class dx12_render_target_view : public render_target_view
	{
	private:
		D3D12_CPU_DESCRIPTOR_HANDLE m_dx_cpu_handle;
	private:
		dx12_render_target_view(D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, const resource_info& res_info);
		virtual void release_impl(device*) override {}
		friend class dx12_device;
	};

	class dx12_depth_stencil_view : public depth_stencil_view
	{
	private:
		D3D12_CPU_DESCRIPTOR_HANDLE m_dx_cpu_handle;
	private:
		dx12_depth_stencil_view(D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle);
		virtual void release_impl(device*) override {}
		friend class dx12_device;
	};

	class dx12_vertex_buffer_view : public vertex_buffer_view
	{
	private:
		D3D12_VERTEX_BUFFER_VIEW m_dx_vbv;
	private:
		dx12_vertex_buffer_view(D3D12_VERTEX_BUFFER_VIEW vb_view);
		virtual void release_impl(device*) override {}
		friend class dx12_device;
	};

	class dx12_index_buffer_view : public index_buffer_view
	{
	private:
		D3D12_INDEX_BUFFER_VIEW m_dx_ibv;
	private:
		dx12_index_buffer_view(D3D12_INDEX_BUFFER_VIEW index_view);
		virtual void release_impl(device*) override {}
		friend class dx12_device;
	};

	class dx12_sampler_view : public sampler_view
	{
	private:
		D3D12_CPU_DESCRIPTOR_HANDLE m_dx_descriptor_handle;
	private:
		dx12_sampler_view(D3D12_CPU_DESCRIPTOR_HANDLE descriptor);
		virtual void release_impl(device*) override {}
		friend class dx12_device;
	};

	class dx12_shader_resource_view : public shader_resource_view
	{
	private:
		D3D12_GPU_DESCRIPTOR_HANDLE m_dx_gpu_handle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_dx_cpu_handle;

	public:
		dx12_shader_resource_view(D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle);
		virtual void release_impl(device*) override {}
		friend class dx12_device;
	};
}