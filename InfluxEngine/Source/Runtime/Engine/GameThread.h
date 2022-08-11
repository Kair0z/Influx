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
		virtual void OnPreTick() override final;
		virtual void OnTick() override final;
		virtual void OnQuit() override final;

		void BindToRenderThread(WeakRef<RenderThread> renderThread);

		GameThread() = default;

	private:
		Ptr<World> CurrentWorld = nullptr;
		WeakRef<RenderThread> BoundRenderThreadRef;

	private:
		void DispatchInitialize();
		void DispatchUpdate(const float deltaTime);
	};
}


