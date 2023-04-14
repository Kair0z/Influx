#include "renderer_pch.h"
#include "InfluxRenderer.h"

#include "Core/Platform/WindowsPlatform.h"

#if INFLUX_RENDERER_USE_CORE
#include "Core/Singleton/Singleton.h"
#endif

// Graphics...
#include "InfluxGraphics.h"

namespace Influx::Renderer
{
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
}

namespace Influx::Renderer
{
	Result Render()
	{
		// Dispatch work to GPU...
		Graphics::DispatchGraphicsCommands([](const Graphics::RHIGraphicsCommandListHandle& cmdList)
			{
				if (IsAttachedToWindow(nullptr))
				{
					const Vectorf4 clearColour = { 1,0,0,1 };
					Graphics::Commands::ClearSwapchainBackBuffer(cmdList, GlobalState::GetAttachedSwapchain(), clearColour);
				}
			});

		return Result{};
	}

	Result Present()
	{
		// Cannot present if we're not attached to a window...
		if (!IsAttachedToWindow(nullptr))
		{
			return Result{};
		}

		// Present the swapchain back-buffer
		constexpr static bool Vsync = true;

		Graphics::DispatchSwapchainPresent(GlobalState::GetAttachedSwapchain(), { Vsync });

		return Result();
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
		Graphics::RHISwapchainDesc swapchainDesc{};
		swapchainDesc.Buffering		= Graphics::RHISwapchainDesc::EBuffering::Triple;
		swapchainDesc.Dimensions	= Platform::GetClientWindowDimensions<uint32>(window);
		swapchainDesc.WindowHandle	= window;

		Graphics::RHISwapchainHandle out_handle;
		Graphics::CreateSwapchain(swapchainDesc, out_handle);

		GlobalState::SetAttachedSwapchain(out_handle);

		return Result();
	}

	bool IsAttachedToWindow(Platform::WindowHandle window)
	{
		return GlobalState::HasAttachedSwapchain();
	}
}

