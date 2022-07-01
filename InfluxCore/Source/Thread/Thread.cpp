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
				while (!bIsQuit)
				{
					LastTick = Time::Now();
					OnTick();
					MsBetweenTicks = Time::GetMillisecondsBetween<float>(Time::Now(), LastTick);
				}
				OnEnd();
			});
	}

	const std::thread& Thread::GetInternalThreadObject()
	{
		return StdThread;
	}
}

