#pragma once

#ifndef __ENGINE_COMMON_H_
#define __ENGINE_COMMON_H_

namespace Influx
{
	class Logger;
	class Profiler;
	class Memory;

	class Engine final
	{
		Logger* mp_logger;
		Profiler* mp_profiler;
		Memory* mp_memory;

	public:
		Engine() = default;
		Engine(const Engine&) = delete;
		Engine(Engine&&) = delete;
		Engine& operator=(const Engine&) = delete;
		Engine& operator=(Engine&&) = delete;
		~Engine() = default;

	public:
		void Initialize();
	};
}

#endif
