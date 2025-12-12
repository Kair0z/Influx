#include "jobs_pch.h"
#include "influx_jobs.h"

namespace influx::jobs
{
    job_queue::job_queue()
    {
        m_job_id_freelist.reserve(k_max_jobs);
        for (uint32 i = 0u; i < k_max_jobs; ++i)
        {
            m_job_id_freelist.push_back(i);
        }
    }

    job_id job_queue::allocate_id()
    {
        for (uint32 i = 0u; i < k_invalid_job; ++i)
        {
            
        }
        return k_invalid_job;
    }

    void job_queue::dealloc_id(const job_id& id)
    {
        
    }

    result<job_id> job_queue::create_job(const job_create_args& args)
    {
        using result_type = result<job_id>;

        const job_id new_id = allocate_id();
        if (new_id == k_invalid_job)
            return result_type::make_error("allocate_id() failed!");

        return new_id;
    }

    result<> job_queue::set_dependency(const job_id source, const job_id dest)
    {
        using result_type = result<>;

        return result_type::make_success();
    }

    result<> job_queue::queue_job(const job_id job)
    {
        using result_type = result<>;

        return result_type::make_success();
    }
}

