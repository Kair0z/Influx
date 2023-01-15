#include "engine_pch.h"
#include "InfluxEngine/Engine.h"

namespace Influx
{
	Engine::Ptr Engine::Create(const Engine::ConstructArgs& args)
	{
		if (EngineLocator::Get() != nullptr)
		{
			return EngineLocator::Get();
		}

		uint64 si = sizeof(Engine);
		EngineLocator::Provide(new Engine(args));

		return EngineLocator::Get();
	}

	Engine::Engine(const Engine::ConstructArgs& args)
		: m_constructionArguments{ args }
	{
		Initialize();
	}

	Engine::~Engine()
	{
		Cleanup();
	}

	void Engine::Destroy(Engine::Ptr engine)
	{
		if (engine != nullptr)
		{
			delete engine;
			engine = nullptr;
		}
	}

	MemoryManager* Engine::GetMemoryManager()
	{
		if (EngineLocator::Get() == nullptr)
		{
			return nullptr;
		}

		return EngineLocator::Get()->mp_mainMemory;
	}

	void Engine::Tick()
	{

		++m_frame;
	}

	

	void Engine::Initialize()
	{

	}

	void Engine::Cleanup()
	{

	}
}

