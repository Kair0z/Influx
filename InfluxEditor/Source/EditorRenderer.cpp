#include "EditorRenderer.h"

#include "ThirdParty/ImGui/imgui.h"
#include "ThirdParty/ImGui/imgui_impl_win32.h"
#include "ThirdParty/ImGui/imgui_impl_dx12.h"

#include "Runtime/Application/WindowsApp.h"

namespace Influx::Editor
{
	void D3D12EditorRenderer::InitializeRHI(const Graphics::GraphicsAPI* gfxApi)
	{
		const Graphics::D3D12API* d3d12Api = Cast<Graphics::D3D12API>(gfxApi);

		// Create a descriptorHeap for the ImGui Font Resources
		FontDescriptorHeap = gfxApi->CreateDescriptorHeap(Graphics::ERHIDescriptorType::Resource, 1, true);
		Graphics::D3D12DescriptorHeap* d3d12DescriptorHeap = Cast<Graphics::D3D12DescriptorHeap>(FontDescriptorHeap);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		ImGui::StyleColorsDark();

		ImGui_ImplWin32_Init(ApplicationLocator::Get()->GetWindow()->GetWindowsHandle());
		ImGui_ImplDX12_Init(d3d12Api->GetDxDevice(), Graphics::RHISwapChain::NumBackBuffers, Graphics::Conversion::ToDx12(Graphics::ERHIFormat::RGBA_8_Unorm), 
			d3d12DescriptorHeap->GetDxDescriptorHeap(), 
			d3d12DescriptorHeap->GetDxDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(), 
			d3d12DescriptorHeap->GetDxDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
	}

	void D3D12EditorRenderer::OnRender(Graphics::RHICommandList* cmdList, Graphics::RHITexture* gameRenderTexture)
	{
		Graphics::D3D12CommandList* d3d12CmdList = Cast<Graphics::D3D12CommandList>(cmdList);

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();

		ImGui::Render();

		cmdList->BindRenderTarget(gameRenderTexture->GetRenderTargetView());
		cmdList->BindDescriptorheap(FontDescriptorHeap);
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), d3d12CmdList->GetDxCommandList());
	}
}
