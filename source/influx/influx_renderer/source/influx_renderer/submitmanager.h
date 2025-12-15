#pragma once
#include "influx_renderer/common.h"

namespace influx::graphics
{
	class commandlist;
	class device;
	class queue;
	class fence;
}

namespace influx::renderer
{
	class gpu_submission final
	{
	public:
		graphics::commandlist* m_commandlist;
		graphics::commandlist& get_commandlist()
		{
			return *m_commandlist;
		}
	};

	enum class e_gpusubmit : uint8
	{
		frame_begin,
		render,
		frame_end,
		pre_present
	};
	static constexpr uint32 k_num_submissions_per_frame = static_cast<uint32>(e_gpusubmit::frame_end) + 1;
	static constexpr uint32 k_num_child_submissions = 32u;
	static const char* k_submit_names[k_num_submissions_per_frame + 1]
	{
		"begin_frame",
		"render",
		"end_frame",
		"pre_present"
	};

	class submit_manager final
	{
		static constexpr uint64 k_num_main_submissions = k_num_submissions_per_frame + 1;
		static constexpr uint64 k_num_submissions_total = k_num_main_submissions * k_num_child_submissions;

		gpu_submission m_submissions[k_num_submissions_total];
		uint32 m_active_submissions[k_num_submissions_total];

		uint64 m_cpu_frame = 0u;
		uint64 m_last_started_gpu_frame = 0u;
		uint64 m_last_finished_gpu_frame = 0u;

		graphics::fence* m_frame_fence = nullptr;
		graphics::fence* m_gpu_finished_fence = nullptr;
		graphics::queue* m_graphics_queue = nullptr;
		graphics::queue* m_copy_queue = nullptr;

		uint64 get_flat_submit_index(uint32 main_submission, uint32 child_index) const
		{
			return (main_submission * k_num_child_submissions) + child_index;
		}

	public:
		submit_manager(graphics::device& device);

		void shutdown(graphics::device& device);

		gpu_submission& get_submission(e_gpusubmit submit, uint32 child_index);

		void wait_until_gpu_idle() const;

		uint64 get_cpu_frame() const;

		uint64 query_gpu_frame() const;

		void wait_until_gpu_frame_finished(const uint64 frame);

		void wait_until_last_gpu_frame_finished()
		{
			wait_until_gpu_frame_finished(m_last_started_gpu_frame);
			m_last_finished_gpu_frame = m_last_started_gpu_frame;
		}

		void wait_until_complete(const gpu_submission& submission);

		void submit_gpu_frame();

		void submit_pre_present();

		void submit(const gpu_submission& submission);

		void submit(const e_gpusubmit submit, uint32 child_index);

		graphics::queue& get_graphics_queue()
		{
			return *m_graphics_queue;
		}

		graphics::queue& get_copy_queue()
		{
			return *m_copy_queue;
		}
	};
}