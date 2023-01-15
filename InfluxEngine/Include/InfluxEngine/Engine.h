#pragma once

#ifndef __ENGINE_ENGINE_H_
#define __ENGINE_ENGINE_H_

#include "InfluxEngine/Common.h"

#include "InfluxEngine/Memory/Object.h"

namespace Influx
{
	class MemoryManager;

	class Engine final : public IObject
	{
		using Ptr = Engine*;

	public:
		struct ConstructArgs final {};
		static Ptr Create(const ConstructArgs& args);
		static void Destroy(Ptr engine);

		static MemoryManager* GetMemoryManager();

		void Tick();

	private:
		MemoryManager* mp_mainMemory;

		Engine() = default;
		Engine(const ConstructArgs& args);

		Engine(const Engine&) = delete;
		Engine(Engine&&) = delete;
		Engine& operator=(const Engine&) = delete;
		Engine& operator=(Engine&&) = delete;
		virtual ~Engine();

		const ConstructArgs m_constructionArguments;

		void Initialize();
		void Cleanup();

		uint64 m_frame;
	};

	using EngineLocator = Locator<Engine>;
}

#endif
