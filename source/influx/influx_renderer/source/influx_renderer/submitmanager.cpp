#include "renderer_pch.h"
#include "submitmanager.h"
#include "renderer_backend.h"

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

			desc.m_type = e_queue_type::copy;
			m_copy_queue = device.create_queue(desc);

			m_frame_fence = device.create_fence(0u);
			m_gpu_finished_fence = device.create_fence(0u);
		}
		
		// create all commandlists up-front
		for (uint32 i = 0u; i < k_num_submissions_total; ++i)
		{
			graphics::commandlist*& cmdlist = m_submissions[i].m_commandlist;
			cmdlist = device.create_graphics_commandlist();
			const uint32 main_index = i / k_num_child_submissions;
			cmdlist->set_name( k_submit_names[main_index] + string("_") + string(i));
		}
	}

	void submit_manager::shutdown(graphics::device& device)
	{
		wait_until_gpu_idle();

		for (uint32 i = 0u; i < k_num_submissions_total; ++i)
		{
			device.release(m_submissions[i].m_commandlist);
			m_submissions[i].m_commandlist = nullptr;
		}
	}

	gpu_submission& submit_manager::get_submission(e_gpusubmit submit, uint32 child_index)
	{
		const uint32 index = static_cast<uint32>(submit);
		return m_submissions[get_flat_submit_index(index, child_index)];
	}

	void submit_manager::wait_until_gpu_idle() const
	{
		m_gpu_finished_fence->queue_signal(1u, m_graphics_queue);
		m_gpu_finished_fence->wait_for_value(1u);
		m_gpu_finished_fence->queue_signal(0u, m_graphics_queue);
	}

	uint64 submit_manager::get_cpu_frame() const
	{
		return m_cpu_frame;
	}

	uint64 submit_manager::query_gpu_frame() const
	{
		return m_frame_fence->query_value();
	}

	void submit_manager::wait_until_gpu_frame_finished(const uint64 frame)
	{
		m_frame_fence->wait_for_value(frame);
	}

	void submit_manager::wait_until_complete(const gpu_submission& submission)
	{
		submission.m_commandlist->wait_for_completion();
	}

	void submit_manager::submit_gpu_frame()
	{
		static constexpr uint32 k_num_submissions_without_present =
			k_num_submissions_total - k_num_child_submissions;

		for (uint32 i = 0u; i < k_num_submissions_without_present; ++i)
		{
			submit(m_submissions[i]);
		}

		m_last_started_gpu_frame++;
		m_frame_fence->queue_signal(m_last_started_gpu_frame, m_graphics_queue);
	}

	void submit_manager::submit_pre_present()
	{
		submit(get_submission(e_gpusubmit::pre_present, 0u));
	}

	void submit_manager::submit(const gpu_submission& submission)
	{
		graphics::commandlist* cmdlist = submission.m_commandlist;
		if (cmdlist == nullptr)
			return;

		// if the commandlist never started recording, don't submit it
		const bool is_recording = cmdlist->is_recording();
		if (!is_recording) 
			return;

		cmdlist->end_recording().get();
		cmdlist->submit(m_graphics_queue).get();
	}

	void submit_manager::submit(const e_gpusubmit sub, uint32 child_index)
	{
		submit(get_submission(sub, child_index));
	}
}