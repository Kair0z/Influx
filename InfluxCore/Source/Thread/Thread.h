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
		
		/* Called first and once in Run() */
		virtual void OnStart() {};

		/* Called right before Tick (to avoid stalling & synchronization messing up the TickMs) */
		/* Contributes to StallMs */
		virtual void OnPreTick() {};

		/* Called */
		virtual void OnTick() {};
		virtual void OnQuit() {};

		const std::thread& GetInternalThreadObject() const;
		uint64_t GetTickCount() const;
		bool IsQuit() const;
		float GetMsSinceLastTick() const;

		void SetQuit();

		virtual ~Thread();

	protected:
		std::thread StdThread;
		std::atomic_bool bIsQuit = false;

	private:
		std::atomic_int64_t TickCount{};
		Time::TimePoint LastTick;
		float TickMs{};
		float StallMs{};
	};
}