#include "pch.h"
#include "Engine.h"

#include "Runtime/Events/EventManager.h"
#include "Runtime/Assets/AssetManager.h"
#include "Runtime/Application/WindowsApp.h"

#include "Runtime/Rendering/RenderThread.h"
#include "Runtime/Engine/GameThread.h"

#include "EngineEvents.h"

#include "Runtime/Logger/Logger.h"

namespace Influx
{
	void Engine::Run()
	{
		Logger::Info("Engine::Pre-Init");
		{
			// Creating Threads & immediately dispatching them:
			mpRenderThread = new RenderThread();
			mpGameThread = new GameThread();
			
			// Create Managers:
			mpAssetManager = AssetManager::Create();
			mpEventManager = EventManager::Create();
			mpApplication = WindowsApp::Create();

			/* Provide locators */
			EventManagerLocator::Provide(mpEventManager);
			ApplicationLocator::Provide(mpApplication);
		}

		Logger::Info("Engine::Init");
		{
			// Run Gamethread & Renderthread
			mpGameThread->Run(*this, *mpRenderThread);
			mpRenderThread->Run(*this);

			/* Subscribe to EventManager for Engine Events */
			mpEventManager->SubscribeToChannel<EventCategory::Engine>([this](Event* e) { OnEvent(e); });
			mpEventManager->SubscribeToChannel<EventCategory::Window>([this](Event* e) { mpRenderThread->OnEvent(e); });
		}

		// Main-thread loop [For now events & Windows Events]
		while (!mIsEngineQuitAtomic)
		{
			// 'Eventthread'
			{
				// [Platform] Poll Windows Events
				mpApplication->PollEvents();

				// [TODO] put separate channel-flushing on separate threads?
				mpEventManager->FlushAllChannels();
			}

			++mCurrentFrame;
		}
	}

	Engine::~Engine()
	{
		delete mpRenderThread;
		delete mpGameThread;
		delete mpAssetManager;
		delete mpEventManager;
		delete mpApplication;
	}

	bool Engine::IsQuit() const
	{
		return mIsEngineQuitAtomic;
	}

	void Engine::OnEvent(const Event* e)
	{
		mIsEngineQuitAtomic = (Cast<EngineQuitEvent>(e) != nullptr);
	}
}

