#pragma once

#include "InfluxRenderer/Renderer.h"

namespace Influx::Graphics
{
	class D3D12Device;
	class D3D12CommandList;
	class D3D12Swapchain;

	class RHIDescriptorHeap;
}

namespace Influx::GUI
{
	class GUIRenderer final : public Renderer::IRenderer
	{
		/* Initializing RHI Resources */
		virtual void Initialize(const RHIDevicePtr device) override;

		/* On attaching to a Window */
		virtual void OnAttachToWindow(const RHIDevicePtr, const RHISwapchainPtr);

		/* Resizing the bound window swapchain */
		virtual void OnSwapchainResize(const RHIDevicePtr, const RHISwapchainPtr,
			const Math::Vectoru2& prevSize, const Math::Vectoru2& newSize) override;

		/* Submitting work onto a passed RHICommandList */
		virtual void OnRender(RHICommandListPtr commandList) const override;

		/* On detaching from a Window */
		virtual void OnDetachFromWindow(const RHIDevicePtr);

		/* Cleaning up RHI Resources */
		virtual void Cleanup(const RHIDevicePtr device) override;

	private:
		void InitializeDx12(Graphics::D3D12Device* devicePtr, Graphics::D3D12Swapchain* swapchainPtr);
		void RenderDx12(Graphics::D3D12CommandList* cmdList) const;
		void CleanupDx12(Graphics::D3D12Device* devicePtr);

		Graphics::RHIDescriptorHeap* mp_fontDescriptorHeap;
	};
}


