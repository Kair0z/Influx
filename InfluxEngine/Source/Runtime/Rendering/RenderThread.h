#pragma once

// RHI Includes:
#include "GraphicsAPI.h"
#include "RHIPipeline.h"
#include "RHIResource.h"

#include <mutex>
#include <thread>
#include <condition_variable>

#include "EditorRenderer.h"

namespace Influx
{
	class RenderFrame;
	class World;
	class Renderer;
	class Engine;

	class RenderThread final : public Thread
	{
	public:
		void OnEvent(const class Event* e);

		virtual void OnStart() override final;
		virtual void OnTick() override final;
		virtual void OnEnd() override final;

		/* [STALL] Stall the calling thread until the renderthread reaches minvalue */
		uint64_t WaitForFrameFinish(uint64_t minValue);

		/* Enqueue a new 'view' capturing the 'render-state' of the current world at the end of Game-thread */
		void EnqueueFrame(const RenderFrame* view);

		RenderThread() = default;
		RenderThread(const RenderThread&) = delete;
		RenderThread(RenderThread&&) = delete;
		RenderThread& operator=(const RenderThread&) = delete;
		RenderThread& operator=(RenderThread&&) = delete;

		inline constexpr static const Vector2u& GetStatGameResolution() { return StatGameResolution; }

	private:
		// RHI Resources:
		Graphics::GraphicsAPI* mpGfxRenderAPI;
		Graphics::RHICommandQueue* mpGfxCommandQueue;
		Graphics::RHISwapChain* mpGfxSwapChain;

		Renderer* mpSceneRenderer;
		Editor::D3D12EditorRenderer* mpEditorRenderer;

		// Game Render Target:
		constexpr static Vector2u StatGameResolution = { 1920, 1080 };
		class Graphics::RHITexture* GameRenderTexture;

		void Initialize();

		Graphics::RHICommandList* BuildRenderCommandList(const Ptr<RenderFrame> frame);
		void SubmitRender(Graphics::RHICommandList* renderCommandList);

		void OnWindowResize(const Vector2u& newSize);

	private:
		/* Frame Value Locks */
		std::mutex mFrameMutex;
		std::condition_variable mFrameConditionVariable;

		/* Renderframe Queue */
		Queue<const RenderFrame*> mRenderFrameQueue;
		std::mutex mRenderViewMutex;
		std::condition_variable mRenderViewCondition;

	private:
		void ShutDown();

		/* [STALL] Pop a Renderframe off the queue. (If no renderframes get submitted, this stalls the renderthread) */
		const Ptr<RenderFrame> RenderThread_ConsumeFrame();
	};
}


