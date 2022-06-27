#pragma once
#include <chrono>

namespace Influx
{
	/* Representation of all Time in the Engine */
	class Time final
	{
	public:
		using TimePoint = std::chrono::system_clock::time_point;

		inline static TimePoint Now()
		{
			return std::chrono::system_clock::now();
		}

		inline void Start()
		{
			mOrigin = Now();
		}

		template <typename T>
		inline static T GetMillisecondsBetween(const TimePoint& end, const TimePoint& start)
		{
			return (T)std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		}

	private:
		TimePoint mOrigin;
	};
}


