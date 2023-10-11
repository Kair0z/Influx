#include "renderer_pch.h"
#include "InfluxRenderer.h"

#include "Core/Platform/WindowsPlatform.h"

#if INFLUX_RENDERER_USE_CORE
#include "Core/Singleton/Singleton.h"
#else
static_assert(false);
#endif

// Graphics...
#include "InfluxGraphics.h"

namespace influx::Renderer
{
	// [GLOBAL STATE]
	struct GlobalState final
		: Singleton<GlobalState>
	{
	public:
		static void SetInitialized(bool initialized)
		{
			Get().m_isInitialized = initialized;
		}
		static bool IsInitialized()
		{
			return Get().m_isInitialized;
		}

		static void SetAttachedSwapchain(Graphics::RHISwapchainHandle handle)
		{
			Get().m_attachedSwapchain = handle;
		}
		static bool HasAttachedSwapchain()
		{
			return Get().m_attachedSwapchain.IsValid();
		}
		static Graphics::RHISwapchainHandle GetAttachedSwapchain()
		{
			return Get().m_attachedSwapchain;
		}

		static uint64& GetFrameIndexReference()
		{
			return Get().m_frameIndex;
		}

		static Graphics::RHICommandListHandle& GetCommandListHandle()
		{
			return Get().m_commandList;
		}

	private:
		Graphics::RHISwapchainHandle m_attachedSwapchain;
		Graphics::RHICommandQueueHandle m_graphicsCommandQueue;

		Graphics::RHICommandListHandle m_commandList;

		bool m_isInitialized;

		uint64 m_frameIndex = 0u;
	};

	Result Render()
	{
		Result result{};

		Graphics::Result gfxResult{};

		// Wait until the Graphics queue finished LAST frame
		const uint64 previousFrame = (GlobalState::GetFrameIndexReference() != 0u) ? GlobalState::GetFrameIndexReference() - 1u : 0u;
		gfxResult = Graphics::WaitForGraphicsQueueSignal(previousFrame);

		gfxResult = Graphics::DispatchGraphicsCommands([](const Graphics::RHICommandListHandle& cmdList)
		{
			if (IsAttachedToWindow(nullptr))
			{
				const Vectorf4 clearColour = { 1,0,0,1 };

				Graphics::Commands::ClearSwapchainBackBuffer(cmdList, GlobalState::GetAttachedSwapchain(), clearColour);
			}
		}
		, previousFrame + 1u);

		// Increase frame value...
		++GlobalState::GetFrameIndexReference();

		return result;
	}

	Result Present()
	{
		Result result{};

		if (!IsAttachedToWindow(nullptr))
		{
			return Result{};
		}

		constexpr static bool Vsync = true;
		Graphics::DispatchSwapchainPresent(GlobalState::GetAttachedSwapchain(), { Vsync });

		return result;
	}

	Result Initialize()
	{
		Result result{};

		Graphics::Initialize(Graphics::EGraphicsAPI::D3D12);
		GlobalState::SetInitialized(true);

		return result;
	}

	bool IsInitialized()
	{
		return GlobalState::IsInitialized();
	}

	Result Cleanup()
	{
		Result result{};

		Graphics::Cleanup();
		GlobalState::SetInitialized(false);
		
		return result;
	}

	Result AttachToWindow(platform::window_handle window)
	{
		Result result{};

		Graphics::RHISwapchainDesc swapchainDesc{};
		swapchainDesc.Buffering		= Graphics::RHISwapchainDesc::EBuffering::Triple;
		swapchainDesc.Dimensions	= platform::GetClientWindowDimensions<uint32>(window);
		swapchainDesc.WindowHandle	= window;

		Graphics::Result gfxResult{};
		Graphics::RHISwapchainHandle out_handle;
		gfxResult = Graphics::CreateSwapchain(swapchainDesc, out_handle);

		GlobalState::SetAttachedSwapchain(out_handle);

		return result;
	}

	bool IsAttachedToWindow(platform::window_handle window)
	{
		return GlobalState::HasAttachedSwapchain();
	}
}

