#pragma once
#include "influx_renderer/common.h"

// influx::core
#include "core/graph/graph.h"

#if WITH_RENDERJOBS
// influx::async
#include "influx_async.h"
#endif

namespace influx::renderer
{
	using job_id = uint32;
	static constexpr job_id k_job_invalid = (job_id)-1;
	static constexpr uint32 k_jobs_capacity = 4 * 1024u;

	struct job_chain final
	{
		job_id m_job_begin = k_job_invalid;
		job_id m_job_end = k_job_invalid;
		uint32 m_num_jobs = 0u;

		void append(job_id id)
		{
			if (m_job_begin == k_job_invalid)
				m_job_begin = id;

			m_num_jobs++;
			m_job_end = id;
		}
	};

	class job_manager final
	{
		using graph = influx::graph<k_jobs_capacity>;
		graph m_graph{};
		job_id m_job_endframe = k_job_invalid;
		async::task_handle m_tasks[k_jobs_capacity];

	public:
		job_manager()
		{
			// initialize
			influx::async::init_args args{};
			args.m_num_workers = 4u;
			args.m_log_callback = nullptr;
			influx::async::initialize(args).get();
		}

		template <typename _jobf>
		job_id create_job(_jobf&& func)
		{
#if !WITH_RENDERJOBS
			func();
			return k_job_invalid;
#else
			async::task_create_args args{};
			args.m_func_execute = func;
			args.m_name = "";
			auto res = influx::async::create_task(args);
			if (!res.is_success())
				return k_job_invalid;

			res.get().dispatch();
#endif
			return k_job_invalid;
		}

		void link(const job_id a, const job_id b)
		{
			if (a == k_job_invalid || b == k_job_invalid)
				return;

			m_graph.set_link(a, b, graph::FORW);
		}
		
		void link(const job_id a, const job_chain& str)
		{
			link(a, str.m_job_begin);
		}

		void link(const job_chain& str, const job_id b)
		{
			link(str.m_job_end, b);
		}

		void link_to_endframe(const job_chain& str)
		{
			link(str, get_endframe_job());
		}

		void link_to_endframe(const job_id a)
		{
			link(a, get_endframe_job());
		}

		void endframe()
		{
			
		}

		job_id get_endframe_job() const
		{
			return m_job_endframe;
		}

		virtual ~job_manager()
		{
			influx::async::shutdown().get();
		}
	};
}