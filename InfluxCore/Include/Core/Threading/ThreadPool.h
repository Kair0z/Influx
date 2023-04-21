#pragma once

#ifndef __CORE_THREADPOOL_H_
#define __CORE_THREADPOOL_H_

#define __CORE_THREADPOOL_USECORE_ 1
#define __CORE_THREADPOOL_USESTL_ 1
#define __CORE_THREADPOOL_USEWINDOWS_ _WIN32

#define __CORE_TODO_ __debugbreak();

#if __CORE_THREADPOOL_USECORE_
#include "Core/BasicTypes.h"
#include "Core/Function.h"
#include "Core/Container/RingBuffer.h"
#else
static_assert(false, "Core_Threadpool UseCore is required...");
#endif

#if __CORE_THREADPOOL_USESTL_
#include <mutex>
#include <functional>
#include <queue>
#else
static_assert(false, "Core_Threadpool UseSTL is required...");
#endif

#if __CORE_THREADPOOL_USEWINDOWS_
#define NOMINMAX
#include <Windows.h>
#else
static_assert(false, "Core_Threadpool UseWindows is required... (for now)");
#endif

namespace Influx
{
    static uint32 GetTotalNumSystemThreads() noexcept
    {
#if __CORE_THREADPOOL_USESTL_
        // https://en.cppreference.com/w/cpp/thread/thread/hardware_concurrency
        return std::thread::hardware_concurrency();
#else
        static_assert(false, "NoImpl!");
#endif
    }

    namespace Internal
    {
        class IThreadPool
        {

        };
    }

    template <uint8 _N>
    class ThreadPool final : public Internal::IThreadPool
    {
    public:
        using Job = std::function<void()>;
        
        constexpr static uint8  k_numThreads = _N;
        constexpr static size_t k_jobCapacity = 256u;

    public:
        ThreadPool()
        {
            m_finishedLabel.store(0);

            const uint32_t max_num = GetTotalNumSystemThreads();
            if (k_numThreads <= max_num)
            {
                for (uint8_t i = 0; i < k_numThreads; ++i)
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

#ifdef __CORE_THREADPOOL_USEWINDOWS_
                    // Windows specific-thread setup:
                    ::HANDLE handle = (::HANDLE)m_threads[i].native_handle();

                    // Put each thread on to dedicated core
                    ::DWORD_PTR affinityMask = 1ull << i;
                    ::DWORD_PTR affinity_result = ::SetThreadAffinityMask(handle, affinityMask);
                    assert(affinity_result > 0);

                    //// Increase thread priority:
                    //BOOL priority_result = SetThreadPriority(handle, THREAD_PRIORITY_HIGHEST);
                    //assert(priority_result != 0);

                    // Name the thread:
                    std::wstringstream wss;

                    wss << "InfluxThreadPool_" << i;
                    ::HRESULT hr = SetThreadDescription(handle, wss.str().c_str());
                    assert(SUCCEEDED(hr));
#endif

                    // m_threads[i].detach();
                }
            }
            else
            {
                __CORE_TODO_;
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
        
        // STALLS the calling thread untill all jobs are finished.
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

        // STALLS the calling thread and terminates all jobs.
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

        uint8 GetNumThreadsWorking()
        {
            __CORE_TODO_;
            return 0u;
        }

        /* For-loop using ThreadPool<_N>::QueueJob() for each iteration */
        /* STALLS the calling thread untill all jobs are finished! */
        static void AsyncFor(Job it_job)
        {
            ThreadPool pool{};
            pool.AsyncFor(it_job);
        }

        void AsyncFor(Job it_job)
        {
            if (it_job == nullptr)
            {
                return;
            }

            for (uint8 i = 0; i < _N; ++i)
            {
                QueueJob(it_job);
            }

            WaitUntilFinished();
        }

        ThreadPool(const ThreadPool&) = delete;
        ThreadPool(ThreadPool&&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;
        ThreadPool& operator=(ThreadPool&&) = delete;
        virtual ~ThreadPool()
        {
            Terminate();
        }

    private:
        bool m_shouldTerminate = false;           // Tells threads to stop looking for jobs

        std::thread m_threads[_N]{};
        Influx::RingBuffer<Job, k_jobCapacity> m_jobs;

        std::mutex m_queueMutex;                  // Prevents data races to the job queue
        std::condition_variable m_muCondition;    // Allows threads to wait on new jobs or termination 

        uint64_t m_currentLabel;
        std::atomic<uint64_t> m_finishedLabel;
    };
}
#endif