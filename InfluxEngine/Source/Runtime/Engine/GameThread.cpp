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

	void GameThread::OnTick()
	{
		const Time::TimePoint preSync = Time::Now();

		// Stall this thread until Renderthread is at max [2] frames behind...
		if (BoundRenderThread)
		{
			BoundRenderThread->WaitForFrameFinish((GetTickCount() > 2) ? GetTickCount() - 3 : 0);
		}
		
		DispatchUpdate(0.0f);

		// Communicating with the Renderthread
		if (BoundRenderThread)
		{
			RenderFrame* frame = RenderFrame::Create(CurrentWorld);
			BoundRenderThread->EnqueueFrame(frame);
		}
	}

	void GameThread::OnEnd()
	{
	}

	void GameThread::BindToRenderThread(RenderThread* renderThread)
	{
		BoundRenderThread = renderThread;
	}

	void GameThread::DispatchInitialize()
	{
		
	}

	void GameThread::DispatchUpdate(const float deltaTime)
	{
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(50ms);
	}

	GameThread::~GameThread()
	{
		bIsQuit = true;
		StdThread.join();

		delete CurrentWorld;
	}
}

