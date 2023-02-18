#include "imgui_pch.h"

#include "Renderer/GUIRenderer.h"
#include "Widgets/ImguiWidget.h"

#include "InfluxGraphics/RHICommandQueue.h"
#include "InfluxGraphics/RHIDescriptorHeap.h"
#include "InfluxGraphics/RHISwapchain.h"
#include "InfluxGraphics/RHICommandList.h"

#include "InfluxGraphics/D3D12/D3D12Device.h"
#include "InfluxGraphics/D3D12/D3D12Swapchain.h"
#include "InfluxGraphics/D3D12/D3D12DescriptorHeap.h"
#include "InfluxGraphics/D3D12/D3D12CommandList.h"

#include "InfluxGraphics/D3D12/ResourceViews/D3D12RenderTargetView.h"

#include "ImGui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include "Core/Platform/WindowsPlatform.h"

#include "ImGui/imgui_impl_dx12.h"

namespace Influx::GUI
{
	GUIRenderer::GUIRenderer(Platform::WindowHandle windowHandle)
		: GUIRenderer()
	{
		AttachToWin32Backend(windowHandle);
	}

	void GUIRenderer::AddWidget(WidgetPtr widget, bool allowDuplicate)
	{
		if (HasWidget(widget) && !allowDuplicate)
		{
			return;
		}

		m_widgetList.push_back(widget);
	}

	void GUIRenderer::RemoveWidget(WidgetPtr widget)
	{
		m_widgetList.remove(widget);
	}

	bool GUIRenderer::HasWidget(WidgetPtr widget) const
	{
		return std::find(m_widgetList.cbegin(), m_widgetList.cend(), widget) != m_widgetList.cend();
	}

	void GUIRenderer::OnInitialize(const DevicePtr)
	{

	}

	void GUIRenderer::OnRender(const CommandListPtr commandList) const
	{
		if (!IsAttachedToRenderTarget())
		{
			return;
		}

		using namespace Graphics;
		D3D12CommandList* d3d12CmdList = (D3D12CommandList*)commandList;
		RenderDx12(d3d12CmdList);
	}

	void GUIRenderer::OnCleanup(const DevicePtr)
	{
		// ...
	}

	void GUIRenderer::OnAttachToRenderTarget(const DevicePtr device, const RenderTargetPtr newRenderTarget)
	{
		using namespace Graphics;
		D3D12Device* d3d12Device		= (D3D12Device*)device;
		D3D12RenderTargetView* d3d12Rtv = (D3D12RenderTargetView*)newRenderTarget;

		InitializeDx12(d3d12Device, d3d12Rtv);
	}

	void GUIRenderer::OnRenderTargetResize(const DevicePtr)
	{
	}

	void GUIRenderer::OnDetachFromRenderTarget(const DevicePtr)
	{
		CleanupDx12();
	}


	void GUIRenderer::InitializeDx12(Graphics::D3D12Device* devicePtr, Graphics::D3D12RenderTargetView* renderTargetView)
	{
		using namespace Graphics;
		
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle{};
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle{};
		if (!devicePtr->GetResourceDescriptorHeap()->GetHandles(cpuHandle, gpuHandle))
		{
			FLX_ASSERT(false);
		}

		ImGui_ImplDX12_Init(devicePtr->GetDxDevice(),
			(int)RHISwapchain::GetNumBackBuffers(),
			Conversion::ToDx12(renderTargetView->GetFormat()),
			nullptr, cpuHandle, gpuHandle);
	}

	
	void GUIRenderer::RenderDx12(Graphics::D3D12CommandList* cmdList) const
	{
		if (!IsAttachedToRenderTarget())
		{
			return;
		}

		// Manually set up display size (every frame to accommodate for window resizing)
		Math::Vectorf2 displaySize = IRenderer::GetCurrentRenderTarget()->GetDimensions();
		ImGui::GetIO().DisplaySize.x = displaySize.x;
		ImGui::GetIO().DisplaySize.y = displaySize.y;

		ImGuiNewFrame();

		//ImGui::ShowDemoWindow();

		for (const WidgetPtr widget : m_widgetList)
		{
			widget->Render();
		}

		ImGuiEndFrame();

		cmdList->BindRenderTarget(GetCurrentRenderTarget());
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList->GetDxCommandList());
	}

	void GUIRenderer::CleanupDx12()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui::DestroyContext();
	}

	void GUIRenderer::ImGuiNewFrame() const
	{
		ImGui_ImplDX12_NewFrame();
		ImGui::NewFrame();
	}

	void GUIRenderer::ImGuiEndFrame() const
	{
		ImGui::Render();
	}

	void GUIRenderer::AttachToWin32Backend(Platform::WindowHandle windowHandle)
	{
		if (IsAttachedToWin32Backend())
		{
			return;
		}

		ImGui_ImplWin32_Init((void*)windowHandle);
	}

	void GUIRenderer::DetachFromWin32Backend()
	{
		ImGui_ImplWin32_Shutdown();
	}

	bool GUIRenderer::IsAttachedToWin32Backend()
	{
		ImGui::CreateContext();
		return ImGui::GetIO().BackendPlatformUserData != nullptr;
	}
}