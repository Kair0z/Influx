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

		void Run(const Engine& engine, RenderThread& rt);

		/* [STALL] Stall the calling thread until the gamethread reaches minvalue */
		uint64_t WaitForFrameFinish(uint64_t minValue);

		float GetMs() const;
		float GetStallMs() const;
		float GetDeltaTime() const;

		GameThread() = default;
		~GameThread();

	private:
		// Internal threadobject
		std::thread mThreadObject;
		std::atomic<uint64_t> mCurrentFrame{};

		Ptr<World> mpCurrentWorld{};

		float Ms{};
		float StallMs{};

		float DeltaTime{};

	private:
		void DoInitialize();
		void DoUpdate(const float deltaTime);

	private:
		void ShutDown();
		void LogInfo(const float ms, const float msWait);
	};
}


