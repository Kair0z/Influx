#pragma once
#include "Memory/Reference.h"
#include "Thread/Thread.h"

#include <mutex>
#include <thread>
#include <condition_variable>

namespace Influx
{
	class AssetManager;
	class EventManager;
	class RenderThread;
	class GameThread;
	class WindowsApp;
	class World;
	class ThreadManager;
	
	class Engine final
	{
	public:
		void Run();

		Engine() = default;
		~Engine();

		bool IsQuit() const;

	private:
		Ptr<AssetManager> mpAssetManager{};
		Ptr<EventManager> mpEventManager{};

		// [Platform] For now .. only Windows
		Ptr<WindowsApp> mpApplication{};
		
		uint64_t mCurrentFrame{};
		std::atomic_bool mIsEngineQuitAtomic{ false };

		Ptr<ThreadManager> ThreadManager;

		// ... MainThread
		Ptr<RenderThread> mpRenderThread;
		Ptr<GameThread> mpGameThread;

	private:
		void OnEvent(const class Event* e);
	};
}


