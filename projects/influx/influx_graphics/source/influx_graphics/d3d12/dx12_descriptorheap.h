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

		virtual descriptor_handle allocate() override;

		virtual void free(descriptor_handle handle) override;

	private:
		ID3D12DescriptorHeap* mpdx_heap;
		uint64 m_descriptor_stride;
		list<void*> m_freelist = {};

		void clear();
	};
}