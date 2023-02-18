#pragma once

#include "InfluxRenderer/IRenderer.h"
#include "Core/Container/List.h"

namespace Influx::Graphics
{
	class D3D12Device;
	class D3D12CommandList;
	class D3D12RenderTargetView;

	class RHIDescriptorHeap;
}

namespace Influx::GUI
{
	class IWidget;
}

namespace Influx::GUI
{
	class GUIRenderer final : public Renderer::IRenderer
	{
	public:
		using WidgetPtr = Ptr<IWidget>;

	public:
		GUIRenderer() = default;

		/* Initializes ImGui Win32-layer */
		GUIRenderer(Platform::WindowHandle windowHandle);

		void AddWidget(WidgetPtr widget, bool allowDuplicate = false);
		void RemoveWidget(WidgetPtr widget);
		bool HasWidget(WidgetPtr widget) const;

		/* Imgui keeps this state */
		static void AttachToWin32Backend(Platform::WindowHandle windowHandle);
		static void DetachFromWin32Backend();
		static bool IsAttachedToWin32Backend();

	private:
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

		void ImGuiNewFrame() const;
		void ImGuiEndFrame() const;

		using WidgetList = List<WidgetPtr>;
		Graphics::RHIDescriptorHeap* mp_fontDescriptorHeap = nullptr;
		WidgetList m_widgetList{};
	};
}


