#pragma once

#ifndef __CORE_THREADPOOL_H_
#define __CORE_THREADPOOL_H_
#define __CORE_THREADPOOL_USECORE_ 1

#if __CORE_THREADPOOL_USECORE_
#include "Core/BasicTypes.h"
#include "Core/Function.h"
#include "Core/Container/RingBuffer.h"
#include <mutex>
#else
namespace Influx
{
    using uint8 = unsigned char;
}

#include <functional>
#include <mutex>
#include <queue>
#endif

#define _CORE_THREADPOOL_USEWINDOWS _WIN32
#if _CORE_THREADPOOL_USEWINDOWS
#define NOMINMAX
#include <Windows.h>
#endif

namespace Influx
{
    template <uint8_t _N>
    class ThreadPool final
    {
    public:
        using Job = std::function<void()>;
        constexpr static size_t k_jobCapacity = 256u;

    public:
        ThreadPool()
        {
            m_finishedLabel.store(0);

            const uint32_t max_num = std::thread::hardware_concurrency();
            if (_N <= max_num)
            {
                for (uint8_t i = 0; i < _N; ++i)
                {
                    m_threads[i] = std::thread([this]()
                        {
                            Job job;
                    while (!m_shouldTerminate)
                    {
                        if (m_jobs.PopFront(job))
                        {
                            job();
                            m_finishedLabel.fetch_add(1);
                        }
                        else
                        {
                            // no job, put thread to sleep
                            std::unique_lock<std::mutex> lock(m_queueMutex);
                            m_muCondition.wait(lock);
                        }
                    }
                        });

#ifdef _CORE_THREADPOOL_USEWINDOWS
                    // Windows specific-thread setup:
                    HANDLE handle = (HANDLE)m_threads[i].native_handle();

                    // Put each thread on to dedicated core
                    DWORD_PTR affinityMask = 1ull << i;
                    DWORD_PTR affinity_result = SetThreadAffinityMask(handle, affinityMask);
                    assert(affinity_result > 0);

                    //// Increase thread priority:
                    //BOOL priority_result = SetThreadPriority(handle, THREAD_PRIORITY_HIGHEST);
                    //assert(priority_result != 0);

                    // Name the thread:
                    std::wstringstream wss;
                    wss << "InfluxThreadPool_" << i;
                    HRESULT hr = SetThreadDescription(handle, wss.str().c_str());
                    assert(SUCCEEDED(hr));
#endif

                    // m_threads[i].detach();
                }
            }
            else
            {

            }
        }

        void QueueJob(const Job& job)
        {
            ++m_currentLabel;

            while (!m_jobs.PushBack(job))
            {
                Poll();
            }

            m_muCondition.notify_one();
        }
        
        bool IsFinished() const
        {
            // Whenever the main thread label is not reached by the workers, it indicates that some worker is still alive
            return m_finishedLabel.load() < m_currentLabel;
        }
        
        void WaitUntilFinished()
        {
            while (!IsFinished()) 
            { 
                Poll(); 
            }
        }

        void Poll()
        {
            m_muCondition.notify_one(); // wake a worker thread
            std::this_thread::yield();
        }

        void Terminate()
        {
            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_shouldTerminate = true;
            }

            m_muCondition.notify_all();

            for (uint8_t i = 0; i < _N; ++i)
            {
                m_threads[i].join();
            }
        }

        virtual ~ThreadPool()
        {
            Terminate();
        }

        ThreadPool(const ThreadPool&) = delete;
        ThreadPool(ThreadPool&&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;
        ThreadPool& operator=(ThreadPool&&) = delete;

    private:
        bool m_shouldTerminate = false;           // Tells threads to stop looking for jobs

        std::thread m_threads[_N]{};
        Influx::RingBuffer<Job, k_jobCapacity> m_jobs;

        std::mutex m_queueMutex;                  // Prevents data races to the job queue
        std::condition_variable m_muCondition;    // Allows threads to wait on new jobs or termination 

        uint64_t m_currentLabel;
        std::atomic<uint64_t> m_finishedLabel;
    };

    /* For-loop using ThreadPool<_N>::QueueJob() for each iteration */
    /* STALLS the calling thread untill all jobs are finished! */
    template <uint8 _N>
    void AsyncFor(Function<void()> function)
    {
        if (function == nullptr)
        {
            return;
        }

        ThreadPool<_N> threadPool{};

        for (uint8 i = 0; i < _N; ++i)
        {
            threadPool.QueueJob(function);
        }

        threadPool.WaitUntilFinished();
    }
}
#endif