#pragma once 
#include "influx_graphics/fence.h"
#include "influx_graphics/d3d12/dx12_base.h"

struct ID3D12Fence;

namespace influx::graphics
{
	class dx12_fence final : public fence
	{
	private:
		ID3D12Fence* mpdx_fence;

	private:
		dx12_fence(ID3D12Fence* fence);
		virtual void release_impl(device*) override;
		friend class dx12_device;

		// queues a signal command to the command queue
		virtual void queue_signal(uint64 value, queue* queue) override;

		virtual void signal(uint64 value) override;

		virtual void wait_for_value(uint64 value) override;
		virtual void wait_for_value(uint64 value, wait_handle& handle) override;

		virtual uint64 query_value() const override;
	};
}