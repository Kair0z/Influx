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
	void RenderThread::Run(const Engine& engine)
	{
		mCurrentFrame = 0;

		// Initialize Render Resources 
		Initialize();

		mThreadObject = std::thread([this, &engine]()
		{
			while (!engine.IsQuit())
			{
				Time::TimePoint preSync = Time::Now();

				/* Stalls if no renderview is submitted on GameThread... */
				const Ptr<RenderFrame> frameToRender = RenderThread_ConsumeFrame();

				// Render
				Time::TimePoint preRender = Time::Now();
				if (frameToRender)
				{
					Graphics::RHICommandList* renderCmdList = BuildRenderCommandList(frameToRender);
					SubmitRender(renderCmdList);
				}
				Time::TimePoint postRender = Time::Now();

				++mCurrentFrame;

				Ms = Time::GetMillisecondsBetween<float>(postRender, preRender);
				StallMs = Time::GetMillisecondsBetween<float>(preRender, preSync);
				// LogInfo(Ms, StallMs);

				// Signal one ::WaitForFrameFinish Candidate (Game Thread)
				mFrameConditionVariable.notify_one();
			}
		});
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
		GfxRenderAPI = &Graphics::D3D12API::Get();

		// Create Command Queue
		GfxCommandQueue = GfxRenderAPI->CreateCommandQueue(Graphics::ERHICommandQueueType::Graphics);

		// Create Window-swapchain from Application Window handle
		void* currentWindowHandle = ApplicationLocator::Get()->GetWindow()->GetWindowsHandle();
		ASSERT(currentWindowHandle != nullptr);
		GfxSwapChain = GfxRenderAPI->CreateSwapChain((HWND)currentWindowHandle, GfxCommandQueue);

		// Create Scene Renderer:
		SceneRenderer = Renderer::Create(GfxRenderAPI);

		// Create Game Render Target:
		Graphics::RHITextureDescription textureDescription{};
		textureDescription.Format = Graphics::ERHIFormat::RGBA_8_Unorm;
		textureDescription.Height = GfxSwapChain->GetHeight();
		textureDescription.Width = GfxSwapChain->GetWidth();
		textureDescription.InitialResourceState = Graphics::ERHIResourceState::RenderTarget;
		textureDescription.OptimizedClearValue = { 1.0f, 0.0f, 0.0f, 1.0f };
		GameRenderTexture = GfxRenderAPI->CreateTexture(textureDescription);
	}

	Graphics::RHICommandList* RenderThread::BuildRenderCommandList(const Ptr<RenderFrame> frame)
	{
		/* Get a new Command List */
		Ptr<Graphics::RHICommandList> gfxCmdList = GfxCommandQueue->SetupNewCommandList(GfxRenderAPI);

		/* Clear Game Render Texture (And force transition the resource to RenderTarget) */
		gfxCmdList->ClearTextureAsRTV(GameRenderTexture, true);

		/* Render Scene to Command List */
		SceneRenderer->Render(gfxCmdList, GameRenderTexture);

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

	float RenderThread::GetMs() const
	{
		return Ms;
	}

	float RenderThread::GetStallMs() const
	{
		return StallMs;
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
		auto isValid = [&minValue, this]{return mCurrentFrame >= minValue; };
		mFrameConditionVariable.wait(lock, isValid);

		return mCurrentFrame;
	}

	void RenderThread::ShutDown()
	{
		mRenderViewCondition.notify_one();
		mThreadObject.join();

		// Flush commandqueue
		GfxCommandQueue->Flush();

		// Delete resources...
		delete SceneRenderer;
		delete GameRenderTexture;
		delete GfxCommandQueue;
		delete GfxSwapChain;
	}

	void RenderThread::LogInfo(const float msBetweenFrames, const float msWaitForGT)
	{
		static float averageTime{};
		constexpr static int updateTimeLogIntv = 1;
		averageTime += msBetweenFrames;

		static float averageWaitTime{};
		averageWaitTime += msWaitForGT;

		if (mCurrentFrame % updateTimeLogIntv == 0)
		{
			averageTime /= updateTimeLogIntv;
			averageWaitTime /= updateTimeLogIntv;
			Logger::Info("RenderThread: [ms: {}][FPS: {}] - [Wait for GT: {}]", averageTime, 1.0f / averageTime * 1000.0f, averageWaitTime);
			averageTime = 0.0f;
			averageWaitTime = 0.0f;
		}
	}
}

