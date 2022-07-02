#pragma once
#include <thread>
#include "../Time/Timer.h"
#include "../Container/Vector.h"
#include <mutex>
#include <thread>
#include <condition_variable>

namespace Influx
{
	class Thread
	{
	public:
		void Run();
		virtual void OnStart() {};
		virtual void OnTick() {};
		virtual void OnEnd() {};

		const std::thread& GetInternalThreadObject() const;
		uint64_t GetTickCount() const;
		bool IsQuit() const;
		float GetMsBetweenTicks() const;

		virtual ~Thread();

	private:
		std::thread StdThread;
		bool bIsQuit = false;
		uint64_t TickCount{};

		Time::TimePoint LastTick;
		float TickMs{};
		float StallMs{};
	};
}