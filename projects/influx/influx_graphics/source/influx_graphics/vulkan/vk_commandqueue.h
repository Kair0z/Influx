#pragma once
#include "influx_graphics/commandqueue.h"
#include "vk_headers.h"

namespace influx::graphics
{
	class vk_commandqueue final : public command_queue
	{
	public:
		explicit vk_commandqueue(const command_queue_desc& desc, const vk::Queue& vkqueue);

		virtual void submit_commandlists(const vector<command_list*>& commandlists) override;

		// queues a signal to the target fence
		virtual void queue_signal(fence* fence, uint64 value) override;

	private:
		vk::Queue m_vk_queue;
	};
}