#include "pch.h"
#include "RenderThread.h"

// Engine Includes:
#include "Runtime/Application/WindowsApp.h"
#include "Runtime/Rendering/RenderFrame.h"
#include "Runtime/Engine/Engine.h"
#include "Runtime/Rendering/Renderer.h"

// RHI Includes:
#if FLX_RENDERER_VULKAN
#include "VulkanAPI.h"
#elif FLX_RENDERER_D3D12
#include "D3D12API.h"
#endif

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

	void RenderThread::OnPreTick()
	{
		/* Stalls if no renderview is submitted on GameThread... */
		mpCurrentRenderFrame = RenderThread_ConsumeFrame();
	}

	void RenderThread::OnTick()
	{
		if (mpCurrentRenderFrame)
		{
			using namespace std::chrono_literals;
			//std::this_thread::sleep_for(ms);

			// Render:
			Graphics::RHICommandList* renderCmdList = BuildRenderCommandList(mpCurrentRenderFrame);
			SubmitRender(renderCmdList);
		}

		Logger::Info("RT{}, ms: {}", GetTickCount(), GetMsSinceLastTick());
	}

	void RenderThread::OnQuit()
	{
		mRenderViewCondition.notify_one();

		// Flush commandqueue
		mpGfxCommandQueue->Flush();

		// Delete resources...
		delete mpSceneRenderer;
		delete GameRenderTexture;
		delete mpGfxCommandQueue;
		delete mpGfxSwapChain;
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
#if FLX_RENDERER_VULKAN
		mpGfxRenderAPI = &Graphics::VulkanAPI::Get();
#elif FLX_RENDERER_D3D12
#if DEBUG
		Graphics::D3D12API::EnableDebugLayer();
#endif
		mpGfxRenderAPI = &Graphics::D3D12API::Get();
#endif

		// Create Command Queue
		mpGfxCommandQueue = mpGfxRenderAPI->CreateCommandQueue(Graphics::ERHICommandQueueType::Graphics);

		// Create Window-swapchain from Application Window handle & Instance
		HWND currentWindowHandle = ApplicationLocator::Get()->GetWindow()->GetWindowsHandle();
		HINSTANCE currentWindowsInstance = ApplicationLocator::Get()->GetInstanceHandle();
		ASSERT(currentWindowHandle != nullptr);
		ASSERT(currentWindowsInstance != nullptr);
		mpGfxSwapChain = mpGfxRenderAPI->CreateSwapChain(currentWindowsInstance, currentWindowHandle, mpGfxCommandQueue);

		return;

		// Create Scene Renderer:
		mpSceneRenderer = new Renderer();
		mpSceneRenderer->InitializeRHI(mpGfxRenderAPI);

		mpEditorRenderer = new Editor::D3D12EditorRenderer();
		mpEditorRenderer->InitializeRHI(mpGfxRenderAPI);

		// Create Main Game Render Target:
		Graphics::RHITextureDescription textureDescription{};
		textureDescription.Format = Graphics::ERHIFormat::RGBA_8_Unorm;
		textureDescription.Height = mpGfxSwapChain->GetHeight();
		textureDescription.Width = mpGfxSwapChain->GetWidth();
		textureDescription.InitialResourceState = Graphics::ERHIResourceState::RenderTarget;
		textureDescription.OptimizedClearValue = { 1.0f, 0.0f, 0.0f, 1.0f };
		GameRenderTexture = mpGfxRenderAPI->CreateTexture(textureDescription);
	}

	Graphics::RHICommandList* RenderThread::BuildRenderCommandList(const Ptr<RenderFrame> frame)
	{
		/* Get a new Command List */
		Ptr<Graphics::RHICommandList> gfxCmdList = mpGfxCommandQueue->SetupNewCommandList(mpGfxRenderAPI);

		/* Clear Game Render Texture (And force transition the resource to RenderTarget) */
		gfxCmdList->ClearTextureAsRTV(GameRenderTexture, true);

		/* Render Scene to Command List */
		mpSceneRenderer->OnRender(gfxCmdList, GameRenderTexture);
		mpEditorRenderer->OnRender(gfxCmdList, GameRenderTexture);

		/* Copy Game Render Texture into current Window-backbuffer (And force transitions on their respective resources) */
		gfxCmdList->CopyResource(GameRenderTexture->GetRHIResource(), mpGfxSwapChain->GetCurrentBackBufferResource(), true);

		return gfxCmdList;
	}

	void RenderThread::SubmitRender(Graphics::RHICommandList* renderCmdList)
	{
		/* Execute the Graphics Command List */
		mpGfxCommandQueue->ExecuteCommmandList(renderCmdList);

		/* Present Window Swapchain */
		mpGfxSwapChain->Present(mpGfxCommandQueue, true);
	}

	void RenderThread::EnqueueFrame(const RenderFrame* view)
	{
		if (IsQuit()) return;
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
			mRenderViewCondition.wait_for(lock, 1ms);
			if (IsQuit()) return nullptr;
		}

		const RenderFrame* frameToRender = mRenderFrameQueue.front();
		mRenderFrameQueue.pop();

		return Ptr<RenderFrame>(frameToRender);
	}

	uint64_t RenderThread::WaitForFrameFinish(uint64_t minValue)
	{	
		// mIsFrame's 'check-for-validness' only happens when the conditional variable gets notified in the Renderthread
		while (GetTickCount() < minValue)
		{
			if (IsQuit()) return minValue;

			using namespace std::chrono_literals;
			std::this_thread::sleep_for(0.1ms);
		}

		return GetTickCount();
	}
}

