#include "pch.h"
#include "GameThread.h"
#include "Runtime/World/World.h"

#include "Runtime/Engine/Engine.h"
#include "Runtime/Rendering/RenderThread.h"
#include "Runtime/Rendering/RenderFrame.h"
#include "Runtime/Logger/Logger.h"

namespace Influx
{
	void GameThread::Run(const Engine& engine, RenderThread& rt)
	{
		mCurrentFrame = 0;

		DoInitialize();

		mThreadObject = std::thread([this, &engine, &rt]()
			{
				Time::TimePoint lastTime = Time::Now();

				while (!engine.IsQuit())
				{
					const Time::TimePoint preSync = Time::Now();

					// Game thread waits for the Renderthread to be at max [2] frames behind
					rt.WaitForFrameFinish((mCurrentFrame > 2) ? uint64_t(mCurrentFrame) - 3 : 0);

					const Time::TimePoint preUpdate = Time::Now();
					DoUpdate(DeltaTime);
					const Time::TimePoint postUpdate = Time::Now();

					// Update MS variables
					DeltaTime = Time::GetTimeBetween<float>(preUpdate, lastTime) / 1000.0f;
					Ms = Time::GetTimeBetween<float>(postUpdate, preUpdate);
					StallMs = Time::GetTimeBetween<float>(preUpdate, preSync);

					lastTime = preUpdate;

					// Communicating with the Renderthread
					{
						RenderFrame* frame = RenderFrame::Create(mpCurrentWorld);
						rt.EnqueueFrame(frame);
					}
					
					++mCurrentFrame;
				}
			});
	}

	void GameThread::DoInitialize()
	{
		mpCurrentWorld = World::Create();
	}

	void GameThread::DoUpdate(const float deltaTime)
	{
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(50ms);
	}

	void GameThread::ShutDown()
	{
		delete mpCurrentWorld;
	}

	void GameThread::LogInfo(const float ms, const float msWait)
	{
		static float averageTime{};
		static float averageWaitTime{};
		constexpr static int updateTimeLogIntv = 1;
		averageTime += ms;
		averageWaitTime += msWait;

		if (mCurrentFrame % updateTimeLogIntv == 0)
		{
			averageTime /= updateTimeLogIntv;
			averageWaitTime /= updateTimeLogIntv;

			Logger::Info("GameThread: [ms: {}][FPS: {}] - [Wait for RT: {}]", averageTime, 1.0f / averageTime * 1000.0f, averageWaitTime);
			averageTime = 0.0f;
			averageWaitTime = 0.0f;
		}
	}

	uint64_t GameThread::WaitForFrameFinish(uint64_t minValue)
	{
		return mCurrentFrame;
	}

	float GameThread::GetDeltaTime() const
	{
		return DeltaTime;
	}

	float GameThread::GetMs() const
	{
		return Ms;
	}

	float GameThread::GetStallMs() const
	{
		return StallMs;
	}

	GameThread::~GameThread()
	{
		mThreadObject.join();
		ShutDown();
	}
}

