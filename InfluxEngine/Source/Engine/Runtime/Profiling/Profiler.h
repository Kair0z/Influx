#pragma once

#include "Core/Time.h"
#include "Core/Singleton/Singleton.h"

namespace Influx
{
	class Profiler final : public Singleton<Profiler>
	{
	public:
		Profiler();
		Profiler(const Profiler&) = delete;
		Profiler(Profiler&&) = delete;
		Profiler& operator=(const Profiler&) = delete;
		Profiler& operator=(Profiler&&) = delete;
		virtual ~Profiler();

	private:

	};
}


