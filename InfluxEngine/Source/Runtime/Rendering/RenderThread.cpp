#include "pch.h"
#include "RenderThread.h"

// Engine Includes:
#include "Runtime/Application/WindowsApp.h"
#include "Runtime/Rendering/RenderFrame.h"
#include "Runtime/Engine/Engine.h"
#include "Runtime/Rendering/Renderer.h"

// RHI Includes:
#include "D3D12API.h"

// Misc Includes:
#include "Runtime/Application/WindowEvents.h"
#include "Runtime/Logger/Logger.h"
#include <chrono>

namespace Influx
{
	void RenderThread::OnStart()
	{
		Initialize();

	}

	void RenderThread::OnTick()
	{
		/* Stalls if no renderview is submitted on GameThread... */
		const Ptr<RenderFrame> frameToRender = RenderThread_ConsumeFrame();
		if (frameToRender)
		{
			// Render:
			Graphics::RHICommandList* renderCmdList = BuildRenderCommandList(frameToRender);
			SubmitRender(renderCmdList);
		}

		// Signal one ::WaitForFrameFinish Candidate (Game Thread)
		mFrameConditionVariable.notify_one();
	}

	void RenderThread::OnEnd()
	{
		
	}

	void RenderThread::OnEvent(const Event* e)
	{
		const WindowResizeEvent* asWindowResize = Cast<WindowResizeEvent>(e);
		if (asWindowResize)
		{
			OnWindowResize({ asWindowResize->NewWidth, asWindowResize->NewHeight });
		}
	}

	void RenderThread::OnWindowResize(const Vector2u& newSize)
	{
		//GfxSwapChain->Resize(GfxRenderAPI, GfxCommandQueue, newSize);
	}

	void RenderThread::Initialize()
	{
#if DEBUG
		Graphics::D3D12API::EnableDebugLayer();
#endif
		mpGfxRenderAPI = &Graphics::D3D12API::Get();

		// Create Command Queue
		GfxCommandQueue = mpGfxRenderAPI->CreateCommandQueue(Graphics::ERHICommandQueueType::Graphics);

		// Create Window-swapchain from Application Window handle
		void* currentWindowHandle = ApplicationLocator::Get()->GetWindow()->GetWindowsHandle();
		ASSERT(currentWindowHandle != nullptr);
		GfxSwapChain = mpGfxRenderAPI->CreateSwapChain((HWND)currentWindowHandle, GfxCommandQueue);

		// Create Scene Renderer:
		mpSceneRenderer = new Renderer();
		mpSceneRenderer->InitializeRHI(mpGfxRenderAPI);

		// Create Game Render Target:
		Graphics::RHITextureDescription textureDescription{};
		textureDescription.Format = Graphics::ERHIFormat::RGBA_8_Unorm;
		textureDescription.Height = GfxSwapChain->GetHeight();
		textureDescription.Width = GfxSwapChain->GetWidth();
		textureDescription.InitialResourceState = Graphics::ERHIResourceState::RenderTarget;
		textureDescription.OptimizedClearValue = { 1.0f, 0.0f, 0.0f, 1.0f };
		GameRenderTexture = mpGfxRenderAPI->CreateTexture(textureDescription);
	}

	Graphics::RHICommandList* RenderThread::BuildRenderCommandList(const Ptr<RenderFrame> frame)
	{
		/* Get a new Command List */
		Ptr<Graphics::RHICommandList> gfxCmdList = GfxCommandQueue->SetupNewCommandList(mpGfxRenderAPI);

		/* Clear Game Render Texture (And force transition the resource to RenderTarget) */
		gfxCmdList->ClearTextureAsRTV(GameRenderTexture, true);

		/* Render Scene to Command List */
		mpSceneRenderer->OnRender(gfxCmdList, GameRenderTexture);

		/* Copy Game Render Texture into current Window-backbuffer (And force transitions on their respective resources) */
		gfxCmdList->CopyResource(GameRenderTexture->GetRHIResource(), GfxSwapChain->GetCurrentBackBufferResource(), true);

		return gfxCmdList;
	}

	void RenderThread::SubmitRender(Graphics::RHICommandList* renderCmdList)
	{
		/* Execute the Graphics Command List */
		GfxCommandQueue->ExecuteCommmandList(renderCmdList);

		/* Present Window Swapchain */
		GfxSwapChain->Present({ true });
	}

	void RenderThread::EnqueueFrame(const RenderFrame* view)
	{
		if (!view) return;

		/* Lock the Renderviewqueue mutex in order to write uninterrupted */
		std::lock_guard<std::mutex> lock(mRenderViewMutex);
		mRenderFrameQueue.push(view);
		mRenderViewCondition.notify_one();
	}

	const Ptr<RenderFrame> RenderThread::RenderThread_ConsumeFrame()
	{
		using namespace std::chrono_literals;

		std::unique_lock<std::mutex> lock(mRenderViewMutex);
		while (mRenderFrameQueue.empty())
		{
			mRenderViewCondition.wait(lock);
			std::this_thread::sleep_for(1ms);
		}

		const RenderFrame* frameToRender = mRenderFrameQueue.front();
		mRenderFrameQueue.pop();

		return Ptr<RenderFrame>(frameToRender);
	}

	uint64_t RenderThread::WaitForFrameFinish(uint64_t minValue)
	{	
		// mIsFrame's 'check-for-validness' only happens when the conditional variable gets notified in the Renderthread
		std::unique_lock<std::mutex> lock(mFrameMutex);
		auto isValid = [&minValue, this]{return GetTickCount() >= minValue; };
		mFrameConditionVariable.wait(lock, isValid);

		return GetTickCount();
	}

	RenderThread::~RenderThread()
	{
		mRenderViewCondition.notify_one();

		// Flush commandqueue
		GfxCommandQueue->Flush();

		// Delete resources...
		delete mpSceneRenderer;
		delete GameRenderTexture;
		delete GfxCommandQueue;
		delete GfxSwapChain;
	}
}

