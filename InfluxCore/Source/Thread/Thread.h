#pragma once
#include <thread>
#include "../Time/Timer.h"

namespace Influx
{
	class Thread
	{
	public:
		void Run();
		virtual void OnStart() {};
		virtual void OnTick() {};
		virtual void OnEnd() {};

		const std::thread& GetInternalThreadObject();

	private:
		std::thread StdThread;
		bool bIsQuit = false;
		uint64_t TickCount{};

		Time::TimePoint LastTick;
		float MsBetweenTicks{};
	};
}