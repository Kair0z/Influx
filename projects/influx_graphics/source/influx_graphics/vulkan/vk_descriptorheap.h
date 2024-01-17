#pragma once 
#include "influx_graphics/descriptorheap.h"
#include "core/container/list.h"

namespace influx::graphics
{
	class vk_descriptor_heap final : public descriptor_heap
	{
	public:
		virtual descriptor_handle allocate() override;

		virtual void free(descriptor_handle handle) override;
	};
}