#include "pch.h"
#include "RenderThread.h"

#if WITH_EDITOR
#include "Editor/Editor.h"
#endif

#include "Runtime/Application/WindowsApp.h"
#include "Runtime/Rendering/RenderFrame.h"
#include "Runtime/Engine/Engine.h"

#include "Runtime/RHI/CommandList.h"
#include "Runtime/RHI/RenderAPI.h"
#include "Runtime/RHI/CommandQueue.h"
#include "Runtime/RHI/SwapChain.h"
#include "Runtime/RHI/PipelineBuilder.h"

// [CRINGE] I don't have a generic 'TransitionResource' plan yet, so D3D12-implementation is all we've got...
#include "Runtime/RHI/D3D12/D3D12API.h"
#include "Runtime/RHI/D3D12/D3D12CommandList.h"
#include "Runtime/RHI/D3D12/D3D12SwapChain.h"
#include "Runtime/RHI/D3D12/D3D12RenderTarget.h"
// [CRINGE]

#include "Core/Platform/Windows/WindowsPlatform.h"
#include "Runtime/Application/WindowEvents.h"
#include "Core/Type/Type.h"
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
					Render(frameToRender);
				}
				Time::TimePoint postRender = Time::Now();

				++mCurrentFrame;

				Ms = Time::GetTimeBetween<float>(postRender, preRender);
				StallMs = Time::GetTimeBetween<float>(preRender, preSync);
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
		mpWindowSwapChain->Resize(mpRenderAPI, mpGraphicsCommandQueue, newSize);
	}

	void RenderThread::Initialize()
	{
		// Create Resource-creating RenderAPI
		mpRenderAPI = RenderAPI::Create();
#if DEBUG
		mpRenderAPI->SetupDebugLayer();
#endif
		mpRenderAPI->Initialize();

		// Create Command Queue
		CommandQueueDesc cmdQueueDesc{};
		mpGraphicsCommandQueue = mpRenderAPI->CreateCommandQueue(cmdQueueDesc);

		// Create Window-swapchain from Application Window handle
		void* currentWindowHandle = ApplicationLocator::Get()->GetWindow()->GetWindowsHandle();
		ASSERT(currentWindowHandle != nullptr);

		Rectf winRect = WindowsPlatform::GetWindowClientRect((HWND)currentWindowHandle);
		SwapChainDesc swpDesc{};
		swpDesc.Height = (int)winRect.WH.y;
		swpDesc.Width = (int)winRect.WH.x;
		swpDesc.WindowHandle = currentWindowHandle;
		mpWindowSwapChain = mpRenderAPI->CreateSwapChain(swpDesc, mpGraphicsCommandQueue);

		// LoadPipelineStateObjects();
		LoadRHIResources();
	}

	void RenderThread::LoadPipelineStateObjects()
	{
		GraphicsPipelineBuilder psoBuild{};
		psoBuild.InputLayout = {};
		psoBuild.PSShaderPath = "";
		psoBuild.VSShaderPath = "";
		psoBuild.DepthStencilViewFormat = ERHIFormat::D_32_Float;
		psoBuild.RenderTargetViewFormats = { ERHIFormat::RGBA_8_Unorm, ERHIFormat::RGBA_8_Unorm, ERHIFormat::RGBA_8_Unorm};
		psoBuild.PrimitiveTopologyType = ERHIPrimitiveTopologyType::Triangle;
		mpRenderAPI->CreateGraphicsPipeline(psoBuild);
	}

	void RenderThread::LoadRHIResources()
	{
		mGameView.GameRenderTarget = mpRenderAPI->CreateRenderTarget(GameView::StatGameResolution, ERHIFormat::RGBA_8_Unorm);
		mGameView.GameDepthTarget = mpRenderAPI->CreateDepthStencilTarget(GameView::StatGameResolution, ERHIFormat::D_32_Float);

#if WITH_EDITOR
		// Load Editor RHIResources...
		EditorLocator::Get()->GetRenderer().LoadResources_RenderThread(mpRenderAPI, mGameView.GameRenderTarget);
#endif
	}

	void RenderThread::Render(const Ptr<RenderFrame> frame)
	{
		/* Get a new Command List */
		Ptr<RHIGraphicsCommandList> gfxCmdList = mpGraphicsCommandQueue->GetNewGraphicsCommandList(mpRenderAPI);

		/* [CRINGE] Transition Windows-swapchain RTV to Present. (Only D3D12) */
		{
			ID3D12GraphicsCommandList* d3d12List = Cast<D3D12GraphicsCommandList>(gfxCmdList)->GetD3D12CommandList();
			auto buffer = Cast<D3D12SwapChain>(mpWindowSwapChain)->GetCurrentBackBufferResource();

			/* Transition Window Rendertarget to Present... */
			D3D12API::TransitionResource(d3d12List, buffer,
				D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

		/* Clear Window Swapchain Render Target */
		gfxCmdList->ClearRenderTarget(mpWindowSwapChain->GetCurrentRenderTarget(), { 0.0f, 0.0f, 0.0f, 1.0f });
		gfxCmdList->ClearDepthStencil(mpWindowSwapChain->GetDepthTarget(), 1.0f);

		// Clear Game Render Target
		gfxCmdList->ClearRenderTarget(mGameView.GameRenderTarget, { 1.0f, 1.0f, 0.0f, 1.0f });
		gfxCmdList->ClearDepthStencil(mGameView.GameDepthTarget, 0.0f);

#if WITH_EDITOR
		/* [CRINGE] Transition Rts to copy over editor into final window... */
		{
			ID3D12GraphicsCommandList* d3d12List = Cast<D3D12GraphicsCommandList>(gfxCmdList)->GetD3D12CommandList();

			// Editor Render:
			EditorRenderer& editorRenderer = EditorLocator::Get()->GetRenderer();
			editorRenderer.Render_RenderThread(gfxCmdList, mGameView.GameRenderTarget);

			auto editorRTBuffer = Cast<D3D12RenderTarget>(editorRenderer.GetRenderTarget())->GetBufferResource();
			auto windowRTBuffer = Cast<D3D12SwapChain>(mpWindowSwapChain)->GetCurrentBackBufferResource();

			D3D12API::TransitionResource(d3d12List, editorRTBuffer,
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);

			D3D12API::TransitionResource(d3d12List, windowRTBuffer,
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);

			// Copy Editor into window rendertarget
			gfxCmdList->CopyRenderTarget(editorRenderer.GetRenderTarget(), 
				mpWindowSwapChain->GetCurrentRenderTarget());

			D3D12API::TransitionResource(d3d12List, editorRTBuffer,
				D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}
		
#endif

		/* [CRINGE] Transition swapchain RTV to Present. (Only D3D12) */
		{
			ID3D12GraphicsCommandList* d3d12List = Cast<D3D12GraphicsCommandList>(gfxCmdList)->GetD3D12CommandList();
			auto buffer = Cast<D3D12SwapChain>(mpWindowSwapChain)->GetCurrentBackBufferResource();

			/* Transition Window Rendertarget to Present... */
			D3D12API::TransitionResource(d3d12List, buffer,
				D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);
		}

		/* Execute the Graphics Command List */
		mpGraphicsCommandQueue->ExecuteCommandList(gfxCmdList);

		/* Present Window Swapchain */
		mpWindowSwapChain->Present({ true });
	}



	void RenderThread::EnqueueFrame(const RenderFrame* view)
	{
		if (!view) return;

		/* Lock the Renderviewqueue mutex in order to write uninterrupted */
		std::lock_guard<std::mutex> lock(mRenderViewMutex);
		mRenderFrameQueue.push(view);
		mRenderViewCondition.notify_one();
	}

	void RenderThread::RegisterRenderInterface(RenderInterface* renderInterface)
	{
		mpRenderInterfaces.push_back(renderInterface);
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

		// Delete resources...
		delete mpWindowSwapChain;
		delete mpGraphicsCommandQueue;
		delete mGameView.GameDepthTarget;
		delete mGameView.GameRenderTarget;
		delete mpRenderAPI;
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

