#include "renderer_pch.h"
#include "submitmanager.h"

namespace influx::renderer
{
	submit_manager::submit_manager(graphics::device& device)
	{
		// create GPU queues
		{
			using namespace graphics;
			queue_desc desc{};
			desc.m_type = e_queue_type::graphics;
			desc.m_priority = graphics::e_queue_priority::normal;
			m_graphics_queue = device.create_queue(desc);
		}
		
		// create all commandlists up-front
		for (uint32 i = 0u; i < k_num_submissions_per_frame; ++i)
		{
			graphics::commandlist*& cmdlist = m_submissions[i].m_commandlist;
			cmdlist = device.create_graphics_commandlist();
			cmdlist->start(&device, nullptr);
			cmdlist->set_name(k_submit_names[i]);
		}
	}

	void submit_manager::shutdown(graphics::device& device)
	{
		for (uint32 i = 0u; i < k_num_submissions_per_frame; ++i)
		{
			device.release(m_submissions[i].m_commandlist);
			m_submissions[i].m_commandlist = nullptr;
		}
	}

	gpu_submission& submit_manager::get_submission(e_gpusubmit submit, uint32 frame)
	{
		const uint32 index = static_cast<uint32>(submit);
		return m_submissions[index];
	}

	gpu_submission& submit_manager::get_submission(e_gpusubmit submit)
	{
		return get_submission(submit, m_cpu_frame);
	}

	gpu_submission& submit_manager::get_endframe_submission(uint32 frame)
	{
		return get_submission(e_gpusubmit::frame_end, frame);
	}

	void submit_manager::wait_until_gpu_idle() const
	{
		// todo...
	}

	uint64 submit_manager::get_cpu_frame() const
	{
		return m_cpu_frame;
	}

	uint64 submit_manager::query_gpu_frame() const
	{
		return 0u;
	}

	void submit_manager::wait_until_gpu_frame_finished(const uint64 frame) const
	{
		
	}

	void submit_manager::wait_until_complete(const gpu_submission& submission)
	{
		// todo...
	}

	void submit_manager::submit_gpu_frame()
	{
		for (uint32 i = 0u; i < k_num_submissions_per_frame; ++i)
		{
			submit(m_submissions[i]);
		}
	}

	void submit_manager::submit_pre_present()
	{
		submit(get_submission(e_gpusubmit::pre_present));
	}

	void submit_manager::submit(const gpu_submission& submission)
	{
		graphics::commandlist* cmdlist = submission.m_commandlist;
		if (cmdlist == nullptr) return;
		cmdlist->end().get();
		cmdlist->submit(m_graphics_queue);
	}
}