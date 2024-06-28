#pragma once 
#include "influx_graphics/fence.h"
#include "vk_headers.h"

namespace influx::graphics
{
	class vk_fence final : public fence
	{
	public:
		vk_fence(const vk::Fence& vk_fence);

		// queues a signal command to the command queue
		virtual void queue_signal(uint64 value, command_queue* queue) override;

		virtual void signal(uint64 value) override;

		virtual void wait_for_value(uint64 value, wait_handle& handle) override;

	private:
		vk::Fence m_vk_fence;
	};
}