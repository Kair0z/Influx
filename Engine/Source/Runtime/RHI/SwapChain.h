#pragma once
#include "Core/Memory/Reference.h"
#include "RenderTarget.h"

namespace Influx
{
#pragma region ForwardDeclarations
	class RHIRenderTarget;
	class RHICommandQueue;
	struct PresentDescription;
#pragma endregion

	struct SwapChainDesc final
	{
		void* WindowHandle;
		uint32_t Width;
		uint32_t Height;
	};

	class RHISwapChain
	{
	public:
		virtual void Present(const PresentDescription& presentDesc) = 0;
		virtual Ptr<RHIRenderTarget> GetCurrentRenderTarget() const = 0;
		virtual Ptr<RHIRenderTarget> GetDepthTarget() const = 0;

		/* This will stall the GPU-thread, and will recreate buffer-resources. */
		virtual void Resize(const Ptr<RenderAPI> api, Ptr<RHICommandQueue> cmdQueue, const Vector2u& newSize) = 0;

		inline constexpr uint32_t GetCurrentBackBufferIndex() const noexcept { return CurrentBackBufferIndex; }
		inline static constexpr size_t GetNumFramesInFlight() noexcept { return StatNumBackBuffers; }

		inline const Vector2u GetDimensions() const { return { Desc.Width, Desc.Height }; }

		RHISwapChain(const RHISwapChain&) = delete;
		RHISwapChain(RHISwapChain&&) = delete;
		RHISwapChain& operator=(const RHISwapChain&) = delete;
		RHISwapChain& operator=(RHISwapChain&&) = delete;
		virtual ~RHISwapChain() = default;

	protected:
		RHISwapChain(const SwapChainDesc& desc) : Desc{ desc } {};

		uint32_t CurrentBackBufferIndex;

		// [CRINGE] It just isn't...???
		constexpr static bool StatTearingSupported = false;
		constexpr static size_t StatNumBackBuffers = 3;

		SwapChainDesc Desc;

		Ptr<RHIRenderTarget> BackBufferRenderTargets[StatNumBackBuffers]{};
		Ptr<RHIRenderTarget> BackBufferDepthTarget{};
	};

	struct PresentDescription final
	{
		bool VSync = true;
	};
}


