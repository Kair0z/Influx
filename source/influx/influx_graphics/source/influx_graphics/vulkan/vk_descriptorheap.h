#pragma once 
#include "influx_graphics/descriptorheap.h"
#include "core/container/list.h"

namespace influx::graphics
{
	class vk_descriptor_heap final : public descriptor_heap
	{
	public:
		vk_descriptor_heap(const descriptor_heap::create_args& args);

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
	};
}