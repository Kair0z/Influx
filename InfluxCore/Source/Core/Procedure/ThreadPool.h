#pragma once

#ifndef _CORE_PROCEDURE_H_
#define _CORE_PROCEDURE_H_

#define __CORE_PROCEDURE_USECORE_ 0

#if __CORE_PROCEDURE_USECORE_
#else
#include <functional>
#include <mutex>
#include <queue>
#endif

namespace Influx
{
    template <uint8_t _N>
    class ThreadPool final
    {
    public:
        using Job = std::function<void()>;

    private:
        bool m_shouldTerminate = false;           // Tells threads to stop looking for jobs
        std::mutex m_queueMutex;                  // Prevents data races to the job queue
        std::condition_variable m_muCondition;    // Allows threads to wait on new jobs or termination 

        uint32_t m_threadsInUse = 0;
        std::thread m_threads[_N]{};

        std::queue<std::function<void()>> jobs;

        inline void PoolLoop()
        {
            while (true) 
            {
                std::function<void()> job;
                {
                    std::unique_lock<std::mutex> lock(m_queueMutex);
                    m_muCondition.wait(lock, [this] { return !jobs.empty() || m_shouldTerminate; });
                    if (m_shouldTerminate) { return; }

                    job = jobs.front();
                    jobs.pop();
                }

                job();
            }
        }

    public:
        ThreadPool();
        void QueueJob(const Job& job);
        bool IsFinished();
        void WaitUntilFinished();
        virtual ~ThreadPool();
        void Terminate();

        ThreadPool(const ThreadPool&) = delete;
        ThreadPool(ThreadPool&&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;
        ThreadPool& operator=(ThreadPool&&) = delete;
    };

#pragma region Impl
    template <uint8_t _N>
    inline ThreadPool<_N>::ThreadPool()
    {
        m_threadsInUse = 0;

        const uint32_t max_num = std::thread::hardware_concurrency();
        if (_N <= max_num)
        {
            for (uint8_t i = 0; i < _N; ++i)
            {
                ++m_threadsInUse;
                m_threads[i] = std::thread([this]() { PoolLoop(); });
            }
        }
    }

    template <uint8_t _N>
    inline void ThreadPool<_N>::QueueJob(const ThreadPool<_N>::Job& job)
    {
        if (m_threadsInUse <= 0) return;
        else
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            jobs.push(job);
        }

        m_muCondition.notify_one();
    }

    template <uint8_t _N>
    inline bool ThreadPool<_N>::IsFinished()
    {
        if (m_threadsInUse <= 0) return true;

        bool isFinished = false;
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            isFinished = jobs.empty();
        }

        return isFinished;
    }

    template <uint8_t _N>
    inline void ThreadPool<_N>::WaitUntilFinished()
    {
        if (m_threadsInUse <= 0) return;

        while (IsFinished()) {}
    }

    template <uint8_t _N>
    inline ThreadPool<_N>::~ThreadPool()
    {
        Terminate();
    }

    template <uint8_t _N>
    inline void ThreadPool<_N>::Terminate()
    {
        if (m_threadsInUse <= 0) return;

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