#pragma once
#include <mutex>
#include <thread>
#include <condition_variable>

#include "Core/Container/Queue.h"
#include "Core/Memory/Reference.h"

namespace Influx
{
	class RenderFrame;
	class World;
	class Renderer;
	class Engine;
	class RenderInterface;

	class RenderAPI;
	class RHICommandQueue;
	class RHISwapChain;
	class RHIRenderTarget;

	// [TODO] Create base Thread class along with ThreadManager
	class RenderThread final
	{
	public:
		void Run(const Engine& engine);
		void OnEvent(const class Event* e);

		/* [STALL] Stall the calling thread until the renderthread reaches minvalue */
		uint64_t WaitForFrameFinish(uint64_t minValue);

		/* Enqueue a new 'view' capturing the 'render-state' of the current world at the end of Game-thread */
		void EnqueueFrame(const RenderFrame* view);

		/* Registers a renderinterface class that hooks into renderthread */
		void RegisterRenderInterface(RenderInterface* renderInterface);

		float GetMs() const;
		float GetStallMs() const;

		RenderThread() = default;
		RenderThread(const RenderThread&) = delete;
		RenderThread(RenderThread&&) = delete;
		RenderThread& operator=(const RenderThread&) = delete;
		RenderThread& operator=(RenderThread&&) = delete;
		inline ~RenderThread() { ShutDown(); }

		// [CRINGE] TEmp
		inline constexpr static Vector2u GetStatGameResolution() { return GameView::StatGameResolution; }

	private:
		// RHI Resources:
		Ptr<RenderAPI> mpRenderAPI;
		Ptr<RHICommandQueue> mpGraphicsCommandQueue;
		Ptr<RHISwapChain> mpWindowSwapChain;

		// Game Render Target:
		struct GameView final
		{
			constexpr static Vector2u StatGameResolution = { 1920, 1080 };
			Ptr<RHIRenderTarget> GameRenderTarget;
			Ptr<RHIRenderTarget> GameDepthTarget;
		};
		GameView mGameView;

		float Ms{};
		float StallMs{};

	private:
		std::thread mThreadObject;

		/* Frame Value Locks */
		uint64_t mCurrentFrame;
		std::mutex mFrameMutex;
		std::condition_variable mFrameConditionVariable;

		/* Renderframe Queue */
		Queue<const RenderFrame*> mRenderFrameQueue;
		std::mutex mRenderViewMutex;
		std::condition_variable mRenderViewCondition;

		Vector<RenderInterface*> mpRenderInterfaces;

	private:
		void Initialize();
		void LoadPipelineStateObjects();
		void LoadRHIResources();

		void Render(const Ptr<RenderFrame> frame);

		void LogInfo(const float msBetweenFrames, const float msWaitForGT);
		void OnWindowResize(const Vector2u& newSize);

	private:
		void ShutDown();

		/* [STALL] Pop a Renderframe off the queue. (If no renderframes get submitted, this stalls the renderthread) */
		const Ptr<RenderFrame> RenderThread_ConsumeFrame();
	};
}


