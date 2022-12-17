#pragma once

#ifndef __ENGINE_ENGINE_H_
#define __ENGINE_ENGINE_H_

namespace Influx
{
	class Engine final
	{
	public:
		struct ConstructArgs final
		{

		};

		Engine() = default;
		Engine(const ConstructArgs& args);
		virtual ~Engine();

		Engine(const Engine&) = delete;
		Engine(Engine&&) = delete;
		Engine& operator=(const Engine&) = delete;
		Engine& operator=(Engine&&) = delete;

	private:
		class Logger* mp_logger;
		class Profiler* mp_profiler;
		class Memory* mp_memory;

		const ConstructArgs m_constructionArguments;

		void Initialize();
		void Cleanup();
	};
}

#endif
