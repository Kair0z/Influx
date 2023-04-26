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

namespace Influx::Renderer
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

	private:
		Graphics::RHISwapchainHandle m_attachedSwapchain;
		bool m_isInitialized;
	};

	Result Render()
	{
		Result result{};

		// Dispatch work to GPU...
		Graphics::DispatchGraphicsCommands([](const Graphics::RHICommandListHandle& cmdList)
		{
			if (IsAttachedToWindow(nullptr))
			{
				const Vectorf4 clearColour = { 1,0,0,1 };

				Graphics::Commands::ClearSwapchainBackBuffer(cmdList, GlobalState::GetAttachedSwapchain(), clearColour);
			}
		});

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
		// Dx12
		Graphics::Initialize(Graphics::EGraphicsAPI::D3D12);

		GlobalState::SetInitialized(true);

		return Result{};
	}

	bool IsInitialized()
	{
		return GlobalState::IsInitialized();
	}

	Result Cleanup()
	{
		Graphics::Cleanup();

		GlobalState::SetInitialized(false);
		
		return Result{};
	}

	Result AttachToWindow(Platform::WindowHandle window)
	{
		Result result{};

		Graphics::Result gfxResult{};

		Graphics::RHISwapchainDesc swapchainDesc{};
		swapchainDesc.Buffering		= Graphics::RHISwapchainDesc::EBuffering::Triple;
		swapchainDesc.Dimensions	= Platform::GetClientWindowDimensions<uint32>(window);
		swapchainDesc.WindowHandle	= window;

		Graphics::RHISwapchainHandle out_handle;
		gfxResult = Graphics::CreateSwapchain(swapchainDesc, out_handle);

		GlobalState::SetAttachedSwapchain(out_handle);

		return result;
	}

	bool IsAttachedToWindow(Platform::WindowHandle window)
	{
		return GlobalState::HasAttachedSwapchain();
	}
}

