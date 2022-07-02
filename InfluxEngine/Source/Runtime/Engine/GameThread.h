#pragma once
#include <mutex>
#include <thread>
#include <condition_variable>

#include "Thread/Thread.h"
#include "Memory/Reference.h"

namespace Influx
{
	class World;
	class Engine;
	class RenderThread;

	class GameThread final : public Thread
	{
	public:
		virtual void OnStart() override final;
		virtual void OnTick() override final;
		virtual void OnEnd() override final;

		void BindToRenderThread(RenderThread* renderThread);

		GameThread() = default;
		virtual ~GameThread();

	private:
		Ptr<World> CurrentWorld = nullptr;
		Ptr<RenderThread> BoundRenderThread = nullptr;
	private:
		void DispatchInitialize();
		void DispatchUpdate(const float deltaTime);

	private:
		void ShutDown();
	};
}


