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

#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx12.h"

namespace Influx::GUI
{
	/* Initializing RHI Resources */
	void GUIRenderer::Initialize(const RHIDevicePtr device)
	{
		// ...
	}

	void GUIRenderer::OnAttachToWindow(const RHIDevicePtr device, const RHISwapchainPtr swapchain)
	{
		using namespace Graphics;
		D3D12Device* d3d12Device = (D3D12Device*)device;
		D3D12Swapchain* d3d12Swapchain = (D3D12Swapchain*)swapchain;

		InitializeDx12(d3d12Device, d3d12Swapchain);
	}

	/* Resizing the bound window swapchain */
	void GUIRenderer::OnSwapchainResize(const RHIDevicePtr device, const RHISwapchainPtr swapchain,
		const Math::Vectoru2& prevSize, const Math::Vectoru2& newSize)
	{
		
	}

	/* Submitting work onto a passed RHICommandList */
	void GUIRenderer::OnRender(RHICommandListPtr commandList) const
	{
		if (NeedsSwapchainUpdate())
		{
			return;
		}

		using namespace Graphics;
		D3D12CommandList* d3d12CmdList = (D3D12CommandList*)commandList;
		RenderDx12(d3d12CmdList);
	}

	void GUIRenderer::OnDetachFromWindow(const RHIDevicePtr device)
	{
		using namespace Graphics;
		D3D12Device* d3d12Device = (D3D12Device*)device;
		CleanupDx12(d3d12Device);
	}

	/* Cleaning up RHI Resources */
	void GUIRenderer::Cleanup(const RHIDevicePtr device)
	{
		// ...
	}

	void GUIRenderer::InitializeDx12(Graphics::D3D12Device* devicePtr, Graphics::D3D12Swapchain* swapchainPtr)
	{
		using namespace Graphics;
		mp_fontDescriptorHeap = devicePtr->CreateDescriptorHeap(ERHIDescriptorType::Resource, 1u, true);
		
		D3D12DescriptorHeap* d3d12FontDescHeap = (D3D12DescriptorHeap*)mp_fontDescriptorHeap;

		const uint64 firstFreeSlot = d3d12FontDescHeap->GetFirstFreeSlot();
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
		if (!d3d12FontDescHeap->GetHandles(cpuHandle, gpuHandle))
		{
			FLX_ASSERT(false);
		}

		ImGui::CreateContext();
		ImGui::GetIO();

		bool success = ImGui_ImplWin32_Init((void*)swapchainPtr->GetWindowHandle());
		if (!success)
		{
			FLX_ASSERT(false);
		}
		
		success = ImGui_ImplDX12_Init(devicePtr->GetDxDevice(),
			(int)swapchainPtr->GetNumBackBuffers(), 
			Conversion::ToDx12(swapchainPtr->GetRenderTargetFormat()),
			d3d12FontDescHeap->GetDxDescriptorHeap(), cpuHandle, gpuHandle);

		if (!success)
		{
			FLX_ASSERT(false);
		}
	}

	void GUIRenderer::RenderDx12(Graphics::D3D12CommandList* cmdList) const
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();

		ImGui::NewFrame();

		ImGui::ShowDemoWindow();

		ImGui::Render();

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList->GetDxCommandList());
	}

	void GUIRenderer::CleanupDx12(Graphics::D3D12Device*)
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
}