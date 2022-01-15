#include "pch.h"
#include "Editor.h"

#include "Runtime/RHI/D3D12/D3D12CommandList.h"
#include "Runtime/RHI/D3D12/D3D12RenderTarget.h"
#include "Runtime/RHI/D3D12/D3D12API.h"
#include "Runtime/RHI/SwapChain.h"
#include "Runtime/RHI/RHITypes.h"

#include "Runtime/Application/WindowsApp.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Influx
{
	// [CRINGE] This should probably be inside Dx12 API...
	size_t EditorRenderer::SRVDescriptorHandleIncrementSize = 0;

	void EditorRenderer::LoadResources_RenderThread(const Ptr<RenderAPI> api, const Ptr<RHIRenderTarget> gameRenderTarget)
	{
		D3D12API* d3d12API = Cast<D3D12API>(api);
		ID3D12Device* device = d3d12API->GetDevice();

		// Get Windows-Window from our Application:
		void* currentWindowHandle = ApplicationLocator::Get()->GetWindow()->GetWindowsHandle();
		WindowsApp::AddWindowEventProcHandler(ImGui_ImplWin32_WndProcHandler);
		Rectf winRect = WindowsPlatform::GetWindowClientRect((HWND)currentWindowHandle);

		// Dx12-Stuff:
		{
			SRVDescriptorHandleIncrementSize = D3D12API::GetDescriptorHandleIncrementSize_CBV_SRV_UAV(device);

			// Create Descheap & handles
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			desc.NumDescriptors = 2;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mpResourceDescriptorHeap));

			Dx12CreateViewportSRVFromGameRenderTarget(api, gameRenderTarget);
		}

		// ImGui-Setup:
		{
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
			//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

			ImGui::StyleColorsDark();

			ImGui_ImplWin32_Init(currentWindowHandle);
			ImGui_ImplDX12_Init(device, (int)RHISwapChain::GetNumFramesInFlight(), DXGI_FORMAT_R8G8B8A8_UNORM,
				mpResourceDescriptorHeap, mpResourceDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
				mpResourceDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		}

		// Create Editor Render Target
		mpEditorRenderTarget = D3D12RenderTarget::CreateRenderTarget(d3d12API, { winRect.WH.x, winRect.WH.y }, ERHIFormat::RGBA_8_Unorm);
	}

	void EditorRenderer::Render_RenderThread(Ptr<RHIGraphicsCommandList> CmdList, Ptr<RHIRenderTarget> viewportRT) const
	{
		CopyGameRenderTargetToViewportSRV(CmdList, viewportRT);

		// ImGui 'Render' -> Renders all widgets
		RenderWidgets();

		// Render Imgui to Editor Render Target
		Dx12RenderToTarget(CmdList);
	}

	void EditorRenderer::RenderWidgets() const
	{
		// Start the Dear ImGui frame
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// Render Demowindow
		ImGui::ShowDemoWindow();

		// Call Draw
		for (OnEditorRenderCallback callback : OnEditorRenderCallbacks)
		{
			callback();
		}

		// Render Viewportwindow
		RenderViewportWindow();

		ImGui::Render();
	}

	void EditorRenderer::RenderViewportWindow() const
	{
		bool isOpen = true;
		ImGui::Begin("Viewport", &isOpen, ImGuiWindowFlags_AlwaysAutoResize);

		int descriptor_index = 1;
		D3D12_GPU_DESCRIPTOR_HANDLE my_texture_srv_gpu_handle = mpResourceDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		auto gpuHandle = my_texture_srv_gpu_handle.ptr + SRVDescriptorHandleIncrementSize * descriptor_index;

		float viewportSize = 0.85f;
		float imageDimX = (float)mpViewportImageResource->GetDesc().Width;
		float imageDimY = (float)mpViewportImageResource->GetDesc().Height;
		ImGui::Image((ImTextureID)gpuHandle, ImVec2{ imageDimX * viewportSize, imageDimY * viewportSize });

		ImGui::End();
	}

	void EditorRenderer::Dx12CreateViewportSRVFromGameRenderTarget(const Ptr<RenderAPI> renderAPI, const Ptr<RHIRenderTarget> gameRT)
	{
		D3D12API* d3d12API = Cast<D3D12API>(renderAPI);
		ID3D12Device* device = d3d12API->GetDevice();

		// Create Resource:
		D3D12_HEAP_PROPERTIES props;
		memset(&props, 0, sizeof(D3D12_HEAP_PROPERTIES));
		props.Type = D3D12_HEAP_TYPE_DEFAULT;
		props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Alignment = 0;
		desc.Width = gameRT->GetDimensions().x;
		desc.Height = gameRT->GetDimensions().y;
		desc.DepthOrArraySize = gameRT->GetConfig().ArraySize;
		desc.MipLevels = gameRT->GetConfig().MipLevels;
		desc.Format = ToDxgi(gameRT->GetFormat());
		desc.SampleDesc.Count = gameRT->GetConfig().SampleCount;
		desc.SampleDesc.Quality = gameRT->GetConfig().SampleQuality;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = ToD3D12(gameRT->GetConfig().ResourceFlags);
		device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, NULL, IID_PPV_ARGS(&mpViewportImageResource));

		D3D12_CPU_DESCRIPTOR_HANDLE my_texture_srv_cpu_handle = mpResourceDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		my_texture_srv_cpu_handle.ptr += (SRVDescriptorHandleIncrementSize * 1);

		// Create SRV for Resource:
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = ToDxgi(gameRT->GetFormat());
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = gameRT->GetConfig().MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		device->CreateShaderResourceView(mpViewportImageResource, &srvDesc, my_texture_srv_cpu_handle);
	}

	void EditorRenderer::CopyGameRenderTargetToViewportSRV(Ptr<RHIGraphicsCommandList> cmdList, Ptr<RHIRenderTarget> viewportRT) const
	{
		D3D12GraphicsCommandList* dxList = Cast<D3D12GraphicsCommandList>(cmdList);

		// Copy Game RT Buffer -> Viewport SRV Buffer
		auto gameRTBuffer = Cast<D3D12RenderTarget>(viewportRT)->GetBufferResource();

		D3D12API::TransitionResource(dxList->GetD3D12CommandList(), gameRTBuffer,
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);

		// Copy Game RTV into Viewport SRV...
		dxList->GetD3D12CommandList()->CopyResource(mpViewportImageResource, gameRTBuffer);

		D3D12API::TransitionResource(dxList->GetD3D12CommandList(), gameRTBuffer,
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	void EditorRenderer::Dx12RenderToTarget(Ptr<RHIGraphicsCommandList> CmdList) const
	{
		D3D12GraphicsCommandList* dxList = Cast<D3D12GraphicsCommandList>(CmdList);

		// Clear & Set Editor Render Target
		CmdList->ClearRenderTarget(mpEditorRenderTarget, StatEditorClearColour);
		CmdList->SetRenderTarget(mpEditorRenderTarget);

		// Set Editor Descriptor Heap
		dxList->GetD3D12CommandList()->SetDescriptorHeaps(1, &mpResourceDescriptorHeap);

		// ImGui does the rest...
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(),
			Cast<D3D12GraphicsCommandList>(CmdList)->GetD3D12CommandList());
	}


	void EditorRenderer::ShutDown() const
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();

		mpViewportImageResource->Release();
		mpResourceDescriptorHeap->Release();
	}

	Ptr<RHIRenderTarget> EditorRenderer::GetRenderTarget() const
	{
		return mpEditorRenderTarget;
	}

	void EditorRenderer::RegisterOnEditorRenderCallback(const EditorRenderer::OnEditorRenderCallback& callback)
	{
		OnEditorRenderCallbacks.push_back(callback);
	}

	const EditorRenderer& Editor::GetRenderer() const
	{
		return Renderer;
	}

	EditorRenderer& Editor::GetRenderer()
	{
		return Renderer;
	}

	void Editor::RegisterOnEditorRenderCallback(const EditorRenderer::OnEditorRenderCallback& callback)
	{
		GetRenderer().RegisterOnEditorRenderCallback(callback);
	}
}

