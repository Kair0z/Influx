#pragma once 
#include "influx_graphics/descriptorheap.h"
#include <list>

struct ID3D12DescriptorHeap;

namespace influx::graphics
{
	class dx12_descriptor_heap final : public descriptor_heap
	{
	public:
		dx12_descriptor_heap(const descriptor_heap::create_args& args, 
			ID3D12DescriptorHeap* dxheap, uint64 descriptor_stride);

		virtual descriptor_handle allocate_cpu() override;
		virtual descriptor_handle allocate_gpu() override;

		virtual void free_cpu(descriptor_handle handle) override;
		virtual void free_gpu(descriptor_handle handle) override;

		virtual void free_cpu(uint32 at_index) override;
		virtual void free_gpu(uint32 at_index) override;

		virtual uint32 get_heap_index_cpu(descriptor_handle handle) const override;
		virtual uint32 get_heap_index_gpu(descriptor_handle handle) const override;

		virtual void free_all_cpu() override;
		virtual void free_all_gpu() override;

	private:
		ID3D12DescriptorHeap* mpdx_heap;
		uint64 m_descriptor_stride;
		list<void*> m_freelist_cpu = {};
		list<void*> m_freelist_gpu = {};

		void clear_cpu();
		void clear_gpu();

		uint32 gpu_handle_to_index(descriptor_handle handle) const;
		uint32 cpu_handle_to_index(descriptor_handle handle) const;
		descriptor_handle index_to_gpu_handle(uint32 index) const;
		descriptor_handle index_to_cpu_handle(uint32 index) const;
	};
}