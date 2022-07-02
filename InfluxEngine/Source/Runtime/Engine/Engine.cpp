#include "pch.h"
#include "Engine.h"

#include "Runtime/Engine/ThreadManager.h"
#include "Runtime/Events/EventManager.h"
#include "Runtime/Assets/AssetManager.h"
#include "Runtime/Application/WindowsApp.h"

#include "Runtime/Rendering/RenderThread.h"
#include "Runtime/Engine/GameThread.h"
#include "Runtime/Events/EventThread.h"

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
			mpThreadManager = ThreadManager::Create();
			mpAssetManager = AssetManager::Create();
			mpApplication = WindowsApp::Create();

			/* Provide locators */
			ApplicationLocator::Provide(mpApplication);
		}

		Logger::Info("Engine::Init");
		{
			// Run Gamethread & Renderthread
			mpRenderThread = mpThreadManager->CreateAndLaunchThread<RenderThread, EThreads::RenderThread>();
			mpGameThread = mpThreadManager->CreateAndLaunchThread<GameThread, EThreads::GameThread>();
			mpGameThread->BindToRenderThread(mpRenderThread);
			mpEventThread = mpThreadManager->CreateAndLaunchThread<EventThread, EThreads::EventThread>();

			/* Subscribe to EventManager for Engine Events */
			EventManagerLocator::Get()->SubscribeToChannel<EventCategory::Engine>([this](Event* e) { OnEvent(e); });
			EventManagerLocator::Get()->SubscribeToChannel<EventCategory::Window>([this](Event* e) { mpRenderThread->OnEvent(e); });
		}

		// Main-thread loop
		while (!mIsEngineQuitAtomic)
		{
			++mCurrentFrame;
		}
	}

	Engine::~Engine()
	{
		delete mpThreadManager;
		delete mpAssetManager;
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

