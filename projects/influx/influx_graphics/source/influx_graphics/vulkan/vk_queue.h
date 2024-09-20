#pragma once
#include "influx_graphics/queue.h"
#include "vk_headers.h"

namespace influx::graphics
{
	class vk_queue final : public queue
	{
	public:
		explicit vk_queue(const queue_desc& desc, const vk::Queue& vkqueue);

		virtual void submit_commandlists(const vector<commandlist*>& commandlists) override;

		// queues a signal to the target fence
		virtual void queue_signal(fence* fence, uint64 value) override;

	private:
		vk::Queue m_vk_queue;
	};
}