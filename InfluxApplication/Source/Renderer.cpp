#include "Renderer.h"

#if PLATFORM_WINDOWS && FLX_APP_RENDERER_D3D12
#include "../ImGui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include "../ImGui/imgui_impl_dx12.h"
#endif

namespace Influx
{
#if PLATFORM_WINDOWS && FLX_APP_RENDERER_D3D12
	ImGuiRendererDx12::ImGuiRendererDx12(const Math::Vectoru2& windowDimensions, ::HWND windowHandle)
	{
		InitializeDx12(windowDimensions, windowHandle);
		InitializeImGui(windowHandle);
		
		InitializeWindowRenderTargets();
	}

	ImGuiRendererDx12::~ImGuiRendererDx12()
	{
		DestroyDx12();
	}


	void ImGuiRendererDx12::RenderFrame(Function<void()> internalRenderFunction)
	{
		// [UI RENDER]
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		internalRenderFunction();

		ImGui::Render();

		// Wait for next Frame Resources:
		auto frameCtx = WaitForNextFrameResources();

		uint backbufferIndex = mp_dxgiSwapchain->GetCurrentBackBufferIndex();
		frameCtx->CommandAllocator->Reset();

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = mp_targetResources[backbufferIndex];
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		mp_dxCommandList->Reset(frameCtx->CommandAllocator, NULL);
		mp_dxCommandList->ResourceBarrier(1, &barrier);

		mp_dxCommandList->ClearRenderTargetView(m_windowTargetDescriptors[backbufferIndex], m_clearColour.Data(), 0, NULL);
		mp_dxCommandList->OMSetRenderTargets(1u, &m_windowTargetDescriptors[backbufferIndex], FALSE, NULL);
		mp_dxCommandList->SetDescriptorHeaps(1u, &mp_dxSrvDescHeap);

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), mp_dxCommandList);

		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		mp_dxCommandList->ResourceBarrier(1u, &barrier);

		mp_dxCommandList->Close();

		// Exe CmdQueue
		mp_dxCommandQueue->ExecuteCommandLists(1u, (ID3D12CommandList* const*)&mp_dxCommandList);

		// [PRESENT]
		mp_dxgiSwapchain->Present(k_useVSync ? 1u : 0u, 0u);

		uint64 fenceValue = m_fenceLastSignaledValue + 1u;
		mp_dxCommandQueue->Signal(m_fence, fenceValue);
		m_fenceLastSignaledValue = fenceValue;
		frameCtx->FenceValue = fenceValue;
	}


	void ImGuiRendererDx12::InitializeImGui(::HWND windowHandle)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

		UpdateImGuiIO();

		ImGui_ImplWin32_Init(windowHandle);

		ImGui_ImplDX12_Init(mp_dxDevice2, k_numFramesInFlight,
			DXGI_FORMAT_R8G8B8A8_UNORM, mp_dxSrvDescHeap,
			mp_dxSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
			mp_dxSrvDescHeap->GetGPUDescriptorHandleForHeapStart());
	}

	void ImGuiRendererDx12::InitializeDx12(const Math::Vectoru2& windowDimensions, ::HWND windowHandle)
	{
		using namespace Influx::Graphics;

#if FLX_APP_RENDERER_DEBUG
		D3D12::EnableDxDebugLayer();
#endif

		// Create Factory -> Adapter -> Device
		mp_dxgiFactory4 = D3D12::CreateDxgiFactory4();
		mp_currentAdapter4 = D3D12::GetDxgiAdapter4(mp_dxgiFactory4, k_useWarp);
		mp_dxDevice2 = D3D12::CreateDxDevice2(mp_currentAdapter4);

		// Create CmdQueue
		constexpr D3D12_COMMAND_LIST_TYPE cmdListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
		mp_dxCommandQueue = D3D12::CreateDxCommandQueue(mp_dxDevice2, cmdListType);

		// Create Descriptor-heaps
		mp_dxSrvDescHeap = D3D12::CreateDxDescriptorHeap(mp_dxDevice2, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1u, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		mp_dxRtvDescHeap = D3D12::CreateDxDescriptorHeap(mp_dxDevice2, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, k_numFramesInFlight);

		// Create Window-Render-Target descriptors
		SIZE_T rtvDescriptorSize = mp_dxDevice2->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mp_dxRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
		for (uint8_t i = 0; i < k_numFramesInFlight; ++i)
		{
			m_windowTargetDescriptors[i] = rtvHandle;
			rtvHandle.ptr += rtvDescriptorSize;
		}

		// Create Per Frame Allocators + 1 Cmdlist
		for (uint8_t i = 0; i < k_numFramesInFlight; ++i)
		{
			m_frameContexts[i].CommandAllocator = D3D12::CreateDxCommandAllocator(mp_dxDevice2, cmdListType);
		}
			

		mp_dxCommandList = D3D12::CreateDxCommandList(mp_dxDevice2, m_frameContexts[0].CommandAllocator, cmdListType);
		mp_dxCommandList->Close();

		// Create Fence
		m_fence = D3D12::CreateDxFence(mp_dxDevice2);
		m_fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

		// Create Swapchain
		mp_dxgiSwapchain = D3D12::CreateDxgiSwapChain(mp_dxgiFactory4, windowHandle, mp_dxCommandQueue, windowDimensions.x, windowDimensions.y, k_numFramesInFlight);
	}

	void ImGuiRendererDx12::InitializeWindowRenderTargets()
	{
		for (uint8_t i = 0; i < k_numFramesInFlight; ++i)
		{
			ID3D12Resource* backbuffer = nullptr;
			mp_dxgiSwapchain->GetBuffer(i, IID_PPV_ARGS(&backbuffer));
			mp_dxDevice2->CreateRenderTargetView(backbuffer, NULL, m_windowTargetDescriptors[i]);
			mp_targetResources[i] = backbuffer;
		}
	}

	void ImGuiRendererDx12::SetClearColour(const Math::Vectorf4& clearColour)
	{
		m_clearColour = clearColour;
	}

	const Math::Vectorf4& ImGuiRendererDx12::GetClearColour() const
	{
		return m_clearColour;
	}

	void ImGuiRendererDx12::DestroyDx12()
	{

	}

	void ImGuiRendererDx12::DestroyDx12RenderTargets()
	{
		WaitForLastSubmittedFrame();

		for (UINT i = 0; i < k_numFramesInFlight; i++)
			if (mp_targetResources[i]) { mp_targetResources[i]->Release(); mp_targetResources[i] = nullptr; }
	}


	ImGuiRendererDx12::FrameContext* ImGuiRendererDx12::WaitForNextFrameResources()
	{
		uint64 nextFrameIndex = m_frame + 1;
		m_frame = nextFrameIndex;

		HANDLE waitableObjects[] = { m_swapchainWaitableObject, NULL };
		DWORD numWaitableObjects = 1;

		FrameContext* frameCtx = &m_frameContexts[nextFrameIndex % k_numFramesInFlight];
		UINT64 fenceValue = frameCtx->FenceValue;
		if (fenceValue != 0) // means no fence was signaled
		{
			frameCtx->FenceValue = 0;
			m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
			waitableObjects[1] = m_fenceEvent;
			numWaitableObjects = 2;
		}

		::WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

		return frameCtx;
	}

	void ImGuiRendererDx12::WaitForLastSubmittedFrame()
	{
		FrameContext* frameCtx = &m_frameContexts[m_frame% k_numFramesInFlight];

		UINT64 fenceValue = frameCtx->FenceValue;
		if (fenceValue == 0)
			return; // No fence was signaled

		frameCtx->FenceValue = 0;
		if (m_fence->GetCompletedValue() >= fenceValue)
			return;

		m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
		::WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	void ImGuiRendererDx12::UpdateImGuiIO()
	{
		ImGuiIO& io = ImGui::GetIO();
		io.FontGlobalScale = m_currentImGuiSettings.FontScale;

		ImGui::GetStyle().WindowRounding = m_currentImGuiSettings.WindowRounding ? 1.0f : 0.0f;

		switch (m_currentImGuiSettings.Style)
		{
		default:
		case ImGuiRendererDx12::ImGuiSettings::EStyle::Dark:
			ImGui::StyleColorsDark();
			break;
		}
	}

	LRESULT ImGuiRendererDx12::WindowsProcedure(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
	{
		if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
			return true;

		switch (uMsg)
		{
		case WM_SIZE:
			if (mp_dxDevice2 != nullptr && wParam != SIZE_MINIMIZED)
			{
				WaitForLastSubmittedFrame();
				DestroyDx12RenderTargets();

				HRESULT result = mp_dxgiSwapchain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), 
					DXGI_FORMAT_UNKNOWN, 0);

				assert(SUCCEEDED(result) && "Failed to resize swapchain.");
				InitializeWindowRenderTargets();
			}
			return 0;
		}
	}
#endif
}
