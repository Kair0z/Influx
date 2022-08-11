#include "Thread.h"

namespace Influx
{
	void Thread::Run()
	{
		bIsQuit = false;
		TickCount = 0;

		const Time::TimePoint preSync = Time::Now();

		StdThread = std::thread([&]
			{
				OnStart();
				LastTick = Time::Now();

				while (!bIsQuit)
				{
					LastTick = Time::Now();
					OnTick();
					TickMs = Time::GetMillisecondsBetween<float>(Time::Now(), LastTick);
					++TickCount;
				}
			});
	}

	const std::thread& Thread::GetInternalThreadObject() const
	{
		return StdThread;
	}

	uint64_t Thread::GetTickCount() const
	{
		return TickCount;
	}

	bool Thread::IsQuit() const
	{
		return bIsQuit;
	}

	float Thread::GetMsSinceLastTick() const
	{
		return TickMs;
	}

	void Thread::SetQuit()
	{
		bIsQuit = true;
		OnEnd();
	}

	Thread::~Thread()
	{
		if (StdThread.joinable())
		{
			StdThread.join();
		}
	}
}

