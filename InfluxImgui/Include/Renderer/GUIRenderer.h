#pragma once

#include "InfluxRenderer/IRenderer.h"

namespace Influx::Graphics
{
	class D3D12Device;
	class D3D12CommandList;
	class D3D12RenderTargetView;

	class RHIDescriptorHeap;
}

namespace Influx::GUI
{
	class GUIRenderer final : public Renderer::IRenderer
	{
		virtual void OnInitialize(const DevicePtr) override;
		virtual void OnRender(const CommandListPtr) const override;
		virtual void OnCleanup(const DevicePtr) override;

		virtual void OnAttachToRenderTarget(const DevicePtr device, const RenderTargetPtr newRenderTarget);
		virtual void OnRenderTargetResize(const DevicePtr);
		virtual void OnDetachFromRenderTarget(const DevicePtr);

	private:
		void InitializeDx12(Graphics::D3D12Device* devicePtr, Graphics::D3D12RenderTargetView* renderTargetView);
		void RenderDx12(Graphics::D3D12CommandList* cmdList) const;
		void CleanupDx12();

		Graphics::RHIDescriptorHeap* mp_fontDescriptorHeap;
	};
}


