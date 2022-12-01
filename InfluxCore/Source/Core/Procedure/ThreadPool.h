#pragma once

#ifndef _CORE_PROCEDURE_H_
#define _CORE_PROCEDURE_H_

#define __CORE_PROCEDURE_USECORE_ 1

#if __CORE_PROCEDURE_USECORE_
#include "Core/Container/RingBuffer.h"
#include <mutex>
#include <functional>
#else
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

    private:
        bool m_shouldTerminate = false;           // Tells threads to stop looking for jobs

        std::thread m_threads[_N]{};
        Influx::RingBuffer<Job, k_jobCapacity> m_jobs;

        std::mutex m_queueMutex;                  // Prevents data races to the job queue
        std::condition_variable m_muCondition;    // Allows threads to wait on new jobs or termination 

        uint64_t m_currentLabel;    
        std::atomic<uint64_t> m_finishedLabel;   

    public:
        ThreadPool();

        void QueueJob(const Job& job);
        
        bool IsFinished();
        
        void WaitUntilFinished();

        void Poll();

        void Terminate();

        virtual ~ThreadPool();
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool(ThreadPool&&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;
        ThreadPool& operator=(ThreadPool&&) = delete;
    };

#pragma region Impl
    template <uint8_t _N>
    inline ThreadPool<_N>::ThreadPool()
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

    template <uint8_t _N>
    inline void ThreadPool<_N>::QueueJob(const ThreadPool<_N>::Job& job)
    {
        m_currentLabel += 1;

        while (!m_jobs.PushBack(job)) Poll();

        m_muCondition.notify_one();
    }

    template <uint8_t _N>
    inline bool ThreadPool<_N>::IsFinished()
    {
        // Whenever the main thread label is not reached by the workers, it indicates that some worker is still alive
        return m_finishedLabel.load() < m_currentLabel;
    }

    template <uint8_t _N>
    inline void ThreadPool<_N>::WaitUntilFinished()
    {
        while (!IsFinished()) { Poll(); }
    }

    template <uint8_t _N>
    void ThreadPool<_N>::Poll()
    {
        m_muCondition.notify_one(); // wake a worker thread
        std::this_thread::yield();
    }

    template <uint8_t _N>
    inline ThreadPool<_N>::~ThreadPool()
    {
        Terminate();
    }

    template <uint8_t _N>
    inline void ThreadPool<_N>::Terminate()
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
#pragma endregion
}
#endif