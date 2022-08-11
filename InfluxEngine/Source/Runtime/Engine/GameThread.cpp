#include "pch.h"
#include "GameThread.h"
#include "Runtime/World/World.h"

#include "Runtime/Engine/Engine.h"
#include "Runtime/Rendering/RenderThread.h"
#include "Runtime/Rendering/RenderFrame.h"
#include "Runtime/Logger/Logger.h"

namespace Influx
{
	void GameThread::OnStart()
	{
		CurrentWorld = World::Create();
	}

	void GameThread::OnPreTick()
	{
		// Stall this thread until Renderthread is at max [2] frames behind...
		if (!BoundRenderThreadRef.expired() && !BoundRenderThreadRef.lock()->IsQuit())
		{
			BoundRenderThreadRef.lock()->WaitForFrameFinish((GetTickCount() > 2) ? GetTickCount() - 3 : 0);
		}
	}

	void GameThread::OnTick()
	{
		DispatchUpdate(GetMsSinceLastTick());

		// Enqueue to  the Renderthread
		if (!BoundRenderThreadRef.expired() && !BoundRenderThreadRef.lock()->IsQuit())
		{
			RenderFrame* frame = RenderFrame::Create(CurrentWorld);
			BoundRenderThreadRef.lock()->EnqueueFrame(frame);
		}
	}

	void GameThread::OnQuit()
	{
		delete CurrentWorld;
	}

	void GameThread::BindToRenderThread(WeakRef<RenderThread> renderThread)
	{
		BoundRenderThreadRef = renderThread;
	}

	void GameThread::DispatchInitialize()
	{
		
	}

	void GameThread::DispatchUpdate(const float deltaTime)
	{
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(5ms);

		Logger::Info("GT{}, ms: {}", GetTickCount(), GetMsSinceLastTick());
	}
}

