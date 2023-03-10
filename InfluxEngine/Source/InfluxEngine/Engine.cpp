#include "engine_pch.h"
#include "InfluxEngine/Engine.h"

#include "InfluxEngine/Memory/MemoryManager.h"

namespace Influx
{

	Engine::Engine(const Engine::ConstructArgs& args)
		: m_constructionArguments{ args }
	{
		Initialize();
	}

	Engine::~Engine()
	{
		Cleanup();
	}

	Engine::Ptr Engine::Create(const Engine::ConstructArgs& args)
	{
		return new Engine(args);
	}

	void Engine::Destroy(Engine::Ptr& engine)
	{
		if (engine != nullptr)
		{
			delete engine;
			engine = nullptr;
		}
	}

	void Engine::Initialize()
	{
		mp_mainMemory = new MemoryManager();
		mp_taskThreadPool = new TaskThreadPool();
	}

	void Engine::Cleanup()
	{

	}

	void Engine::Tick()
	{
		++m_frame;
	}

	MemoryManager& Engine::GetMemoryManager()
	{
		return *mp_mainMemory;
	}

	Engine::TaskThreadPool& Engine::GetTaskThreadPool()
	{
		return *mp_taskThreadPool;
	}
}

