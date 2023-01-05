#pragma once

#include "Common.h"

#ifndef __ENGINE_ENGINE_H_
#define __ENGINE_ENGINE_H_

namespace Influx
{
	class Engine final
	{
	public:
		using Ptr = Engine*;

		struct ConstructArgs final
		{

		};

		static Ptr Create(const ConstructArgs& args);
		static void Destroy(Ptr engine);

		void Tick();

	private:
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
}

#endif
