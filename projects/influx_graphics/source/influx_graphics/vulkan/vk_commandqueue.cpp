#include "graphics_pch.h"
#include "influx_graphics/vulkan/vk_commandqueue.h"
#include "influx_graphics/vulkan/vk_commandlist.h"
#include "vk_headers.h"
#include "vk_commandqueue.h"

namespace influx::graphics
{
	vk_commandqueue::vk_commandqueue(const command_queue_desc& desc, const vk::Queue& vkqueue)
		: command_queue(desc)
		, m_vk_queue{vkqueue}
	{
		mp_native = &m_vk_queue;
	}

	void vk_commandqueue::submit_commandlists(const vector<command_list*>& commandlists)
	{
		// get native command buffers
		vector<vk::CommandBuffer*> vkcommandbuffers{};
		for (size_t i = 0u; i < commandlists.size(); ++i)
		{
			vkcommandbuffers.push_back(commandlists[i]->get_native<vk::CommandBuffer>());
		}

		// submit
		vk::SubmitInfo new_info{};
		new_info.commandBufferCount = static_cast<uint32>(commandlists.size());
		new_info.pCommandBuffers;
		m_vk_queue.submit(new_info);
	}
}