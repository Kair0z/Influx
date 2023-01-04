#include "InfluxEngine/Engine.h"

#include "Core/Cleanup.h"

namespace Influx
{
	Engine::Ptr Engine::Create(const Engine::ConstructArgs& args)
	{
		Engine::Ptr newEngine = new Engine(args);

		return newEngine;
	}

	void Engine::Destroy(Engine::Ptr engine)
	{
		if (engine != nullptr)
		{
			delete engine;
			engine = nullptr;
		}
	}

	void Engine::Tick()
	{

		++m_frame;
	}

	Engine::Engine(const Engine::ConstructArgs& args)
		: m_constructionArguments{args}
	{
		Initialize();
	}

	Engine::~Engine()
	{
		Cleanup();
	}
}

