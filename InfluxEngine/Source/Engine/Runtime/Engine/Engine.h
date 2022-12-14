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
	public:
		Engine() = default;
		Engine(const Engine&) = delete;
		Engine(Engine&&) = delete;
		Engine& operator=(const Engine&) = delete;
		Engine& operator=(Engine&&) = delete;
		~Engine();

		struct Global final
		{
			static const Logger* GetLogger();
			static const Profiler* GetProfiler();

		private:
			Global() = delete;

			static Logger* mp_logger;
			static Profiler* mp_profiler;

			friend class Engine;
		};

	private:
		Logger* mp_logger;
		Profiler* mp_profiler;
		Memory* mp_memory;

		void Initialize();
	};
}

#endif
