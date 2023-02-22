#pragma once

#ifndef __ENGINE_ENGINE_H_
#define __ENGINE_ENGINE_H_

#include "InfluxEngine/Common.h"

namespace Influx
{
	class MemoryManager;

	class Engine final
	{
		using Ptr = Engine*;
		using TaskThreadPool = ThreadPool<INFLUX_ENGINE_NUM_TASK_THREADS>;

	public:
		struct ConstructArgs final {};

		Engine() = default;
		Engine(const ConstructArgs& args);
		virtual ~Engine();
		static Ptr Create(const ConstructArgs& args);
		static void Destroy(Ptr& engine);

		MemoryManager& GetMemoryManager();
		TaskThreadPool& GetTaskThreadPool();

		void Tick();

	private:
		uint64 m_frame;

		MemoryManager* mp_mainMemory;
		TaskThreadPool* mp_taskThreadPool;

		const ConstructArgs m_constructionArguments;

		void Initialize();
		void Cleanup();

	public:
		Engine(const Engine&) = delete;
		Engine(Engine&&) = delete;
		Engine& operator=(const Engine&) = delete;
		Engine& operator=(Engine&&) = delete;
	};
}

#endif
