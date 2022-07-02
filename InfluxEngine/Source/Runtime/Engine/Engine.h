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
	class EventThread;
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

		// [Platform] For now .. only Windows
		Ptr<WindowsApp> mpApplication{};
		
		Ptr<ThreadManager> mpThreadManager;

		uint64_t mCurrentFrame{};
		std::atomic_bool mIsEngineQuitAtomic{ false };

		// Thread References
		Ptr<RenderThread> mpRenderThread;
		Ptr<GameThread> mpGameThread;
		Ptr<EventThread> mpEventThread;

	private:
		void OnEvent(const class Event* e);
	};
}


