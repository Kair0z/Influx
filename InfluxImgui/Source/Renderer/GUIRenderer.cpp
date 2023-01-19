#include "imgui_pch.h"

#include "Renderer/GUIRenderer.h"



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
#include "Core/Platform/WindowsPlatform.h"

#include "ImGui/imgui_impl_dx12.h"

namespace Influx::GUI
{
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

		ImGui::CreateContext();
		ImGui::GetIO();

		bool success = ImGui_ImplWin32_Init(Platform::GetCurrentWindowHandle());
		if (!success)
		{
			FLX_ASSERT(false);
		}

		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle{};
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle{};
		if (!devicePtr->GetResourceDescriptorHeap()->GetHandles(cpuHandle, gpuHandle))
		{
			FLX_ASSERT(false);
		}

		success = ImGui_ImplDX12_Init(devicePtr->GetDxDevice(),
			(int)RHISwapchain::GetNumBackBuffers(),
			Conversion::ToDx12(renderTargetView->GetFormat()),
			nullptr, cpuHandle, gpuHandle);

		if (!success)
		{
			FLX_ASSERT(false);
		}
	}

	void GUIRenderer::RenderDx12(Graphics::D3D12CommandList* cmdList) const
	{
		// Set render target?

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();

		ImGui::NewFrame();

		ImGui::ShowDemoWindow();

		ImGui::Render();

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList->GetDxCommandList());
	}

	void GUIRenderer::CleanupDx12()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
}