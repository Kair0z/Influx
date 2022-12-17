#include "Engine.h"

namespace Influx
{
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

