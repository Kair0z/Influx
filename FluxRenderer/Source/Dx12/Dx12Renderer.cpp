#include "Dx12Renderer.h"

#include "InfluxGraphics/D3D12/D3D12.h"
#include "Core/Platform/WindowsPlatform.h"

namespace influx
{
	void Dx12Renderer::RecordRenderCommands(platform::window_handle windowHandle)
	{
		Initialize();
		InitializeSwapchain(windowHandle);

		// Command list allocators can only be reset when the associated 
		// command lists have finished execution on the GPU; apps should use 
		// fences to determine GPU execution progress.
		ID3D12CommandAllocator* commandAllocator = mp_commandAllocators[0u];
		commandAllocator->Reset();

		// We build a command-list for  this frame and the next!
		for (uint8 i = 0u; i < GetNumSwapchainBuffers(); ++i)
		{
			ID3D12GraphicsCommandList* gfxCommandList = mp_gfxCommandLists[i];
			uint8 swapchainBufferIndex = i;

			// However, when ExecuteCommandList() is called on a particular command 
			// list, that command list can then be reset at any time and must be before 
			// re-recording.
			gfxCommandList->Reset(commandAllocator, mp_pipelineState);

			platform::create_window_args windowSettings = platform::get_window_settings(windowHandle);

			D3D12_VIEWPORT viewport{};
			viewport.Width = static_cast<float>(windowSettings.m_width);
			viewport.Height = static_cast<float>(windowSettings.m_height);
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;

			D3D12_RECT scissorRect{};
			scissorRect.left = 0u;
			scissorRect.bottom = 0u;
			scissorRect.right = scissorRect.left + static_cast<LONG>(windowSettings.m_width);
			scissorRect.top = scissorRect.bottom + static_cast<LONG>(windowSettings.m_height);

			gfxCommandList->SetGraphicsRootSignature(mp_rootSignature);
			gfxCommandList->RSSetViewports(1u, &viewport);
			gfxCommandList->RSSetScissorRects(1u, &scissorRect);

			ID3D12Resource* swapchainBufferResource = mp_swapchainBufferResources[swapchainBufferIndex];
			Graphics::D3D12::TransitionResource(gfxCommandList, swapchainBufferResource,
				D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

			// Get the handle off the rtv-heap, and set...
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = Graphics::D3D12::GetDescriptorCpuHandle(mp_rtvDescriptorHeap, swapchainBufferIndex, m_rtvDescriptorSize);
			gfxCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

			// Record commands.
			const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
			gfxCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
			gfxCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			if (mp_vertexBufferResource && mp_indexBufferResource)
			{
				D3D12_VERTEX_BUFFER_VIEW vtbView{};
				vtbView.BufferLocation = mp_vertexBufferResource->GetGPUVirtualAddress();
				vtbView.StrideInBytes = sizeof(Scene::Mesh::Vertex);
				vtbView.SizeInBytes = static_cast<uint32>(GetVertexBufferSize());

				gfxCommandList->IASetVertexBuffers(0, 1, &vtbView);
				gfxCommandList->DrawInstanced(3, 1, 0, 0);
			}

			Graphics::D3D12::TransitionResource(gfxCommandList, swapchainBufferResource,
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

			gfxCommandList->Close();
		}
	}

	void Dx12Renderer::PresentToWindow(platform::window_handle windowHandle)
	{
		InitializeSwapchain(windowHandle);

		for (uint32 i = 0u; i < GetNumSwapchainBuffers(); ++i)
		{
			ID3D12CommandList* commandLists[] = { mp_gfxCommandLists[i] };
			mp_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
			mp_swapchain->Present(0, 0);
		}
		
		WaitForPreviousFrame();
	}

	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
	// sample illustrates how to use fences for efficient resource usage and to
	// maximize GPU utilization.
	void Dx12Renderer::WaitForPreviousFrame()
	{
		// Signal and increment the fence value.
		const UINT64 value = m_fenceValue;
		mp_commandQueue->Signal(mp_fence, value);

		m_fenceValue += 3u;

		// Wait until the previous frame is finished.
		if (mp_fence->GetCompletedValue() < value)
		{
			mp_fence->SetEventOnCompletion(value, m_fenceEvent);

			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	}


	void Dx12Renderer::Initialize()
	{
		InitializeDevice();
		InitializeCommandQueue();
		InitializeDescriptorHeaps();
		InitializeCommandList();
		InitializePipeline();
		InitializeSceneDataBuffers();
		InitializeSynchronization();
	}

	void Dx12Renderer::InitializeDevice()
	{
		if (mp_dxgiFactory2 || mp_dxgiHardwareAdapter || mp_device)
		{
			return;
		}

		const bool debug = true;
		if (debug)
		{
			Graphics::D3D12::EnableDxDebugLayer();
		}
		else
		{
			Graphics::D3D12::DisableDxDebugLayer();
		}
		
		mp_dxgiFactory2	= Graphics::D3D12::Factory::CreateTier2(debug);
		mp_dxgiHardwareAdapter = Graphics::D3D12::Adapter::Select(mp_dxgiFactory2, 0u);
		mp_device = Graphics::D3D12::Device::Create(mp_dxgiHardwareAdapter, debug);
		m_dsvDescriptorSize = mp_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_rtvDescriptorSize = mp_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_resDescriptorSize = mp_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		m_samplerDescriptorSize = mp_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	}

	void Dx12Renderer::InitializeCommandQueue()
	{
		if (mp_commandQueue)
		{
			return;
		}

		mp_commandQueue = Graphics::D3D12::CreateDxCommandQueue(mp_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
	}

	void Dx12Renderer::InitializeSwapchain(platform::window_handle windowHandle)
	{
		if (mp_swapchain != nullptr)
		{
			return;
		}

		platform::create_window_args settings = platform::get_window_settings(windowHandle);

		mp_swapchain = Graphics::D3D12::Swapchain::CreateTier3(mp_dxgiFactory2, (::HWND)windowHandle, mp_commandQueue,
			settings.m_width, settings.m_height, GetNumSwapchainBuffers(), DXGI_FORMAT_R8G8B8A8_UNORM);

		m_currentSwapchainBufferIndex = mp_swapchain->GetCurrentBackBufferIndex();
		
		for (uint8 i = 0u; i < GetNumSwapchainBuffers(); ++i)
		{
			mp_swapchain->GetBuffer(i, IID_PPV_ARGS(&mp_swapchainBufferResources[i]));
			mp_device->CreateRenderTargetView(mp_swapchainBufferResources[i], nullptr, 
				Graphics::D3D12::GetDescriptorCpuHandle(mp_rtvDescriptorHeap, i, m_rtvDescriptorSize));
		}
	}

	void Dx12Renderer::InitializeDescriptorHeaps()
	{
		if (mp_dsvDescriptorHeap == nullptr)
		{
			mp_dsvDescriptorHeap = Graphics::D3D12::CreateDxDescriptorHeap(mp_device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 64u);
		}

		if (mp_rtvDescriptorHeap == nullptr)
		{
			mp_rtvDescriptorHeap = Graphics::D3D12::CreateDxDescriptorHeap(mp_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 64u);
		}

		if (mp_resDescriptorHeap == nullptr)
		{
			mp_resDescriptorHeap = Graphics::D3D12::CreateDxDescriptorHeap(mp_device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 64u);
		}

		if (mp_samplerDescriptorHeap == nullptr)
		{
			mp_samplerDescriptorHeap = Graphics::D3D12::CreateDxDescriptorHeap(mp_device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 16u);
		}
	}

	void Dx12Renderer::InitializeCommandList()
	{
		if (mp_commandAllocators[0u] != nullptr)
			return;

		if (mp_gfxCommandLists[0u] != nullptr)
			return;

		mp_commandAllocators[0u] = Graphics::D3D12::CreateDxCommandAllocator(mp_device, D3D12_COMMAND_LIST_TYPE_DIRECT);

		for (uint8 i = 0u; i < GetNumSwapchainBuffers(); ++i)
		{
			mp_gfxCommandLists[i] = Graphics::D3D12::CreateDxGraphicsCommandList(mp_device, mp_commandAllocators[0u]);
			mp_gfxCommandLists[i]->Close();
		}
	}

	void Dx12Renderer::InitializePipeline()
	{
		Graphics::D3D12::HelperStructs::RootSignatureDesc rootSigDesc{};
		// Default empty...
		mp_rootSignature = Graphics::D3D12::CreateDxSerializedRootSignature(rootSigDesc, mp_device);

		Graphics::D3D12::HelperStructs::GraphicsPipelineStateDesc pipelineDesc{};
		pipelineDesc.AddInputElement("SV_POSITION", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u, 0u, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0u);
		pipelineDesc.AddInputElement("COLOR", 0u, DXGI_FORMAT_R32G32B32A32_FLOAT, 0u, 12u, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0u);
		pipelineDesc.VertexShaderBytecode = GetMaterial().VertexShader;
		pipelineDesc.PixelShaderByteCode = GetMaterial().PixelShader;
		pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		pipelineDesc.DepthStencilState.DepthEnable = false;
		pipelineDesc.DepthStencilState.StencilEnable = false;
		pipelineDesc.SampleMask = 255u;
		pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pipelineDesc.RenderTargetFormats.push_back( DXGI_FORMAT_R8G8B8A8_UNORM );
		pipelineDesc.SampleDesc.Count = 1u;

		mp_pipelineState = Graphics::D3D12::CreateDxGraphicsPipelineState(pipelineDesc, mp_rootSignature, mp_device);
	}

	void Dx12Renderer::InitializeSynchronization()
	{
		if (mp_fence != nullptr)
			return;

		mp_fence = Graphics::D3D12::CreateDxFence(mp_device);
		m_fenceValue = 1u;

		// Create an event handle to use for frame synchronization.
		m_fenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			::HRESULT_FROM_WIN32(::GetLastError());
		}

		WaitForPreviousFrame();
	}

	void Dx12Renderer::InitializeSceneDataBuffers()
	{
		if (mp_vertexBufferResource != nullptr || mp_indexBufferResource != nullptr)
			return;

		// Note: using upload heaps to transfer static data like vert buffers is not 
		// recommended. Every time the GPU needs it, the upload heap will be marshalled 
		// over. Please read up on Default Heap usage. An upload heap is used here for 
		// code simplicity and because there are very few verts to actually transfer.
		constexpr bool useUploadHeap = true;

		vector<Scene::Mesh::Vertex> allVertices{};
		vector<Scene::Mesh::Index> allIndices{};

		for (const Scene::Mesh& mesh : GetMeshes())
		{
			for (const Scene::Mesh::Vertex& vertex : mesh.GetVertices())
				allVertices.push_back(vertex);

			for (const Scene::Mesh::Index& index : mesh.GetIndices())
				allIndices.push_back(index);
		}

		if (allVertices.dimension() == 0u || allIndices.dimension() == 0u)
			return;
		
		auto bufferDesc = Graphics::D3D12::HelperStructs::CommittedResourceDesc::AsBuffer(useUploadHeap, GetVertexBufferSize(), 0u);
		mp_vertexBufferResource = Graphics::D3D12::CreateDxCommittedResource(mp_device, bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ);

		auto idxBufferDesc = Graphics::D3D12::HelperStructs::CommittedResourceDesc::AsBuffer(useUploadHeap, GetIndexBufferSize(), 0u);
		mp_indexBufferResource = Graphics::D3D12::CreateDxCommittedResource(mp_device, bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ);

		Graphics::D3D12::ResourceScopedMap(mp_vertexBufferResource, [&allVertices](void* handle)
			{
				memcpy(handle, allVertices.m_data(), allVertices.dimension() * sizeof(Scene::Mesh::Vertex));
			});

		Graphics::D3D12::ResourceScopedMap(mp_indexBufferResource, [&allIndices](void* handle)
			{
				memcpy(handle, allIndices.m_data(), allIndices.dimension() * sizeof(Scene::Mesh::Index));
			});
	}

	void Dx12Renderer::InitializeShaderResourceDataBuffers()
	{
	}

	void Dx12Renderer::InitializeSamplers()
	{
	}

	void Dx12Renderer::InitializeLights()
	{
	}
}

