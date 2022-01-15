#pragma once
#include "Core/Memory/Reference.h"
#include "Core/Thread/Thread.h"

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
	
	class Editor;

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
		Ptr<Editor> mpEditor{};

		// [Platform] For now .. only Windows
		Ptr<WindowsApp> mpApplication{};
		
		uint64_t mCurrentFrame{};
		std::atomic_bool mIsEngineQuitAtomic{ false };

		// ... MainThread
		Ptr<RenderThread> mpRenderThread;
		Ptr<GameThread> mpGameThread;

#if WITH_EDITOR
		void OnEditorRender() const;
#endif

	private:
		void OnEvent(const class Event* e);
	};
}


