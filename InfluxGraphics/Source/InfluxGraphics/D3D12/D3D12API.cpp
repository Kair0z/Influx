#include "D3D12API.h"
#include "D3D12Resource.h"
#include "D3D12Pipeline.h"
#include "D3D12Conversion.h"
#include "D3D12RenderPass.h"
#include "D3D12Shader.h"

namespace Influx::Graphics
{
	/* API Create Functions */
#pragma region APICreateFunctions
	RHICommandQueue* D3D12API::CreateCommandQueue(const ERHICommandQueueType type) const
	{
		D3D12CommandQueue* result = new D3D12CommandQueue();
		result->eType = type;
		result->DxCommandQueue = CreateDxCommandQueue(DxDevice, Conversion::ToDx12(type));
		result->DxFence = CreateDxFence(DxDevice);
		return result;
	}

	RHISwapChain* D3D12API::CreateSwapChain(HINSTANCE, HWND windowHandle, RHICommandQueue* commandQueue) const
	{
		D3D12SwapChain* result = new D3D12SwapChain();
		D3D12CommandQueue* dxCommandQueue = (D3D12CommandQueue*)commandQueue;

		RECT rect;
		if (GetWindowRect(windowHandle, &rect))
		{
			uint32_t width = static_cast<uint32_t>(rect.right - rect.left);
			uint32_t height = static_cast<uint32_t>(rect.bottom - rect.top);

			result->Width = width;
			result->Height = height;

			result->DxgiSwapChain = CreateDxgiSwapChain(DxgiFactory, windowHandle, dxCommandQueue->DxCommandQueue, width, height, RHISwapChain::NumBackBuffers);
			result->bIsTearingSupported = D3D12API::CheckDxgiTearingSupport();
			result->CurrentBackBufferIndex = result->DxgiSwapChain->GetCurrentBackBufferIndex();

			// Gather Backbuffers & RTVs
			result->DxRenderTargetDescriptorHeap = CreateDescriptorHeap(DxDevice, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, RHISwapChain::NumBackBuffers);

			D3D12_CPU_DESCRIPTOR_HANDLE rtv_cpu_handle(result->DxRenderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
			D3D12_GPU_DESCRIPTOR_HANDLE rtv_gpu_handle = result->DxRenderTargetDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
			
			SIZE_T offsetSize = GetRTVDescriptorSize();
			for (int i = 0; i < RHISwapChain::NumBackBuffers; ++i)
			{
				// Get the buffer resources
				ID3D12Resource* dxBufferResource;
				result->DxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&dxBufferResource));
				result->BackBufferResources[i] = new D3D12Resource(dxBufferResource, ERHIResourceState::Present);
				
				// Create the RenderTargetViews & store
				DxDevice->CreateRenderTargetView(dxBufferResource, nullptr, rtv_cpu_handle);
				D3D12RenderTargetView* d3d12Rtv = new D3D12RenderTargetView();
				d3d12Rtv->DxCPUHandle = rtv_cpu_handle;
				d3d12Rtv->DxGPUHandle = rtv_gpu_handle;
				result->BackBufferRTVs[i] = d3d12Rtv;

				// Offset
				rtv_cpu_handle.ptr = rtv_cpu_handle.ptr + SIZE_T(offsetSize);
				rtv_gpu_handle.ptr = rtv_gpu_handle.ptr + SIZE_T(offsetSize);
			}
		}

		return result;
	}
	
	RHIShader* D3D12API::CreateRHIShader(const std::vector<uint8_t>& fromCompiledData, ERHIShaderType shaderType, ERHIShaderModel shaderModel) const
	{
		D3D12Shader* result = new D3D12Shader(fromCompiledData, shaderType, shaderModel);
		return result;
	}

	RHIDescriptorHeap* D3D12API::CreateDescriptorHeap(const ERHIDescriptorType type, uint32_t numDescriptors, bool shaderVisible) const
	{
		D3D12DescriptorHeap* result = new D3D12DescriptorHeap();
		result->DxDescriptorHeap = D3D12API::CreateDescriptorHeap(DxDevice, Conversion::ToDx12(type), numDescriptors, (shaderVisible) ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
		result->bIsShaderVisible = shaderVisible;
		result->DescriptorStride = GetDescriptorSize(type);
		result->HeapType = type;
		result->NumDescriptors = numDescriptors;
		result->OccupiedSlotIndices = {};
		return result;
	}
#pragma endregion

	/* API-Object Functions */
#pragma region APIObjectFunctions
	D3D12API::D3D12API()
	{
		EnableDebugLayer();

		DxgiFactory = D3D12API::CreateDxgiFactory();
		DxgiAdapter = D3D12API::GetAdapter(DxgiFactory, true);
		DxDevice = D3D12API::CreateDevice(DxgiAdapter);

		CreateGlobalDescriptorHeaps();
	}

	D3D12API::~D3D12API()
	{
		SafeRelease(DxgiAdapter);
		SafeRelease(DxDevice);
		SafeRelease(DxgiFactory);

		delete RTVDescriptorHeap;
		delete ResourceDescriptorHeap;
		delete DSVDescriptorheap;
		delete SamplerDescriptorHeap;

		D3D12API::ReportLiveObjects();
	}

	ID3D12Device2* D3D12API::GetDxDevice() const
	{
		return DxDevice;
	}
	IDXGIAdapter4* D3D12API::GetDxgiAdapter() const
	{
		return DxgiAdapter;
	}
	IDXGIFactory4* D3D12API::GetDxgiFactory() const
	{
		return DxgiFactory;
	}

	const size_t D3D12API::GetRTVDescriptorSize() const
	{
		return CachedRtvDescriptorSize;
	}
	const size_t D3D12API::GetDSVDescriptorSize() const
	{
		return CachedDsvDescriptorSize;
	}
	const size_t D3D12API::GetResourceDescriptorSize() const
	{
		return CachedResourceDescriptorSize;
	}
	const size_t D3D12API::GetSamplerDescriptorSize() const
	{
		return CachedSamplerDescriptorSize;
	}

	const size_t D3D12API::GetDescriptorSize(const ERHIDescriptorType type) const
	{
		switch (type)
		{
		default:
		case ERHIDescriptorType::Invalid:
			return 0;

		case ERHIDescriptorType::DSV:
			return GetDSVDescriptorSize();

		case ERHIDescriptorType::Resource:
			return GetResourceDescriptorSize();

		case ERHIDescriptorType::RTV:
			return GetRTVDescriptorSize();

		case ERHIDescriptorType::Sampler:
			return GetSamplerDescriptorSize();
		}
	}

	void D3D12API::CreateGlobalDescriptorHeaps()
	{
		// Cache DescriptorSizes
		CachedDsvDescriptorSize = DxDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		CachedResourceDescriptorSize = DxDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		CachedSamplerDescriptorSize = DxDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		CachedRtvDescriptorSize = DxDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// Create Global Descriptor Heaps:
		RTVDescriptorHeap = (D3D12DescriptorHeap*)CreateDescriptorHeap(ERHIDescriptorType::RTV, 64);
		DSVDescriptorheap = (D3D12DescriptorHeap*)CreateDescriptorHeap(ERHIDescriptorType::DSV, 64);
		SamplerDescriptorHeap = (D3D12DescriptorHeap*)CreateDescriptorHeap(ERHIDescriptorType::Sampler, 16);
		ResourceDescriptorHeap = (D3D12DescriptorHeap*)CreateDescriptorHeap(ERHIDescriptorType::Resource, 64);
	}

	void D3D12API::CreateDescriptorOnGlobalHeap(ERHIDescriptorType type, size_t slot)
	{
		D3D12DescriptorHeap* heap = nullptr;
		size_t descriptorStride = 0;

		switch (type)
		{
		default:
		case ERHIDescriptorType::DSV:
			heap = DSVDescriptorheap;
			descriptorStride = CachedDsvDescriptorSize;
			break;

		case ERHIDescriptorType::Resource:
			heap = ResourceDescriptorHeap;
			descriptorStride = CachedResourceDescriptorSize;
			break;

		case ERHIDescriptorType::RTV:
			heap = RTVDescriptorHeap;
			descriptorStride = CachedRtvDescriptorSize;
			break;

		case ERHIDescriptorType::Sampler:
			heap = SamplerDescriptorHeap;
			descriptorStride = CachedSamplerDescriptorSize;
			break;
		}

		size_t freeSlot = heap->GetFirstFreeSlot();

		heap->OccupiedSlotIndices.push_back(freeSlot);
	}
#pragma endregion

	/* D3D12CommandList */
#pragma region D3D12CommandList
	void D3D12CommandList::RecordRenderPass(RHIRenderPass* renderPass, const RHIRenderPassBeginInfo& beginInfo, Function<void(RHICommandList* cmdList)> func)
	{
		constexpr bool godKnowsWeShouldReallyNotShipThisUnfinishedMessYet = true;
		ID3D12GraphicsCommandList4* commandList4 = (ID3D12GraphicsCommandList4*)DxCommandList;

		// We run the command list recording as if nothing ever happened
		if (commandList4 == nullptr || godKnowsWeShouldReallyNotShipThisUnfinishedMessYet) func(this); 

		D3D12_CPU_DESCRIPTOR_HANDLE const& rtvCPUDescriptorHandle{};
		D3D12_CPU_DESCRIPTOR_HANDLE const& dsvCPUDescriptorHandle{};

		D3D12_RENDER_PASS_BEGINNING_ACCESS renderPassBeginningAccessClear{ D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR, { /*clearvalue*/ }};
		D3D12_RENDER_PASS_ENDING_ACCESS renderPassEndingAccessPreserve{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE, {} };
		D3D12_RENDER_PASS_RENDER_TARGET_DESC renderPassRenderTargetDesc{ rtvCPUDescriptorHandle, renderPassBeginningAccessClear, renderPassEndingAccessPreserve };

		D3D12_RENDER_PASS_BEGINNING_ACCESS renderPassBeginningAccessNoAccess{ D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS, {} };
		D3D12_RENDER_PASS_ENDING_ACCESS renderPassEndingAccessNoAccess{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS, {} };
		D3D12_RENDER_PASS_DEPTH_STENCIL_DESC renderPassDepthStencilDesc{ dsvCPUDescriptorHandle, renderPassBeginningAccessNoAccess, renderPassBeginningAccessNoAccess, renderPassEndingAccessNoAccess, renderPassEndingAccessNoAccess };

		
		commandList4->BeginRenderPass(static_cast<uint32_t>(renderPass->GetAttachments().size()), 
			&renderPassRenderTargetDesc, &renderPassDepthStencilDesc, D3D12_RENDER_PASS_FLAG_NONE);
		{
			func(this);
		}
		commandList4->EndRenderPass();
	}

	void D3D12CommandList::TransitionResource(RHIResource* resource, const ERHIResourceState newState)
	{
		if (resource->GetCurrentState() == newState) return;

		D3D12Resource* d3d12Resource = (D3D12Resource*)resource;
		ID3D12Resource* dxResource = d3d12Resource->GetDxResource();

		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = dxResource;
		barrier.Transition.StateBefore = Conversion::ToDx12(resource->GetCurrentState());
		barrier.Transition.StateAfter = Conversion::ToDx12(newState);
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		DxCommandList->ResourceBarrier(1, &barrier);

		resource->Transition(newState);
	}

	void D3D12CommandList::ClearRTV(RHIRenderTargetView* renderTargetView, const Math::Vector4f& clearValue)
	{
		D3D12RenderTargetView* d3d12RTV = (D3D12RenderTargetView*)renderTargetView;

		const FLOAT color[4] = { clearValue[0], clearValue[1], clearValue[2], clearValue[3] };
		DxCommandList->ClearRenderTargetView(d3d12RTV->DxCPUHandle, color, 0, nullptr);
	}

	void D3D12CommandList::BindScissorRect(const RHIScissorRect& scissorRect)
	{
		D3D12_RECT dxRect{};
		dxRect.left		= static_cast<LONG>(scissorRect.Left);
		dxRect.top		= static_cast<LONG>(scissorRect.Bottom);
		dxRect.right	= static_cast<LONG>(scissorRect.Width);
		dxRect.bottom	= static_cast<LONG>(scissorRect.Height);

		DxCommandList->RSSetScissorRects(1, &dxRect);
	}

	void D3D12CommandList::BindViewports(const RHIViewport& viewport)
	{
		D3D12_VIEWPORT dxViewport{};
		dxViewport.TopLeftY = viewport.Bottom;
		dxViewport.TopLeftX = viewport.Left;
		dxViewport.Width = viewport.Width;
		dxViewport.Height = viewport.Height;
		dxViewport.MaxDepth = 1.0f;
		dxViewport.MinDepth = 0.0f;

		DxCommandList->RSSetViewports(1, &dxViewport);
	}

	void D3D12CommandList::BindVertexBuffer(RHIVertexBuffer* vertexBuffer)
	{
		D3D12VertexBuffer* d3d12VertexBuffer = (D3D12VertexBuffer*)vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW vtBufferView = d3d12VertexBuffer->GetDxVertexBufferView();
		DxCommandList->IASetVertexBuffers(0, 1, &vtBufferView);
	}

	void D3D12CommandList::SetPrimitiveTopology(ERHIPrimitiveTopology topology)
	{
		DxCommandList->IASetPrimitiveTopology(Conversion::ToDx12(topology));
	}

	void D3D12CommandList::CopyResource(RHIResource* source, RHIResource* dest, bool forceTransition)
	{
		if (forceTransition)
		{
			TransitionResource(source, ERHIResourceState::CopySource);
			TransitionResource(dest, ERHIResourceState::CopyDest);
		}

		DxCommandList->CopyResource(((D3D12Resource*)dest)->GetDxResource(), ((D3D12Resource*)source)->GetDxResource());

		// Force transition back
		if (forceTransition)
		{
			TransitionResource(source, source->GetPreviousState());
			TransitionResource(dest, dest->GetPreviousState());
		}
	}

	void D3D12CommandList::ClearTextureAsRTV(RHITexture* texture, bool forceTransition)
	{
		ClearTextureAsRTV(texture, texture->GetOptimizedClearValue(), forceTransition);
	}

	void D3D12CommandList::ClearTextureAsRTV(RHITexture* texture, const Math::Vector4f& clearValue, bool forceTransition)
	{
		if (forceTransition)
		{
			TransitionResource(texture->GetRHIResource(), ERHIResourceState::RenderTarget);
		}

		ClearRTV(texture->GetRenderTargetView(), clearValue);
	}

	void D3D12CommandList::BindPipelineLayout(RHIGraphicsPipelineLayout* pipelineLayout)
	{
		D3D12GraphicsPipelineLayout* d3d12Layout = (D3D12GraphicsPipelineLayout*)pipelineLayout;
		DxCommandList->SetGraphicsRootSignature(d3d12Layout->GetDxRootSignature());
	}

	void D3D12CommandList::BindPipelineState(RHIGraphicsPipeline* pipeline)
	{
		D3D12GraphicsPipeline* d3d12Pipeline = (D3D12GraphicsPipeline*)pipeline;
		DxCommandList->SetPipelineState(d3d12Pipeline->GetDxPipelineState());
	}

	void D3D12CommandList::BindRenderTarget(RHIRenderTargetView* renderTargetView)
	{
		D3D12RenderTargetView* d3d12Rtv = (D3D12RenderTargetView*)renderTargetView;
		DxCommandList->OMSetRenderTargets(1, &d3d12Rtv->DxCPUHandle, true, nullptr); // Todo DSV?
	}

	void D3D12CommandList::DrawInstanced(uint32_t numVerticesPerInstance, uint32_t numInstances, uint32_t startVertexLocation, uint32_t startInstanceLocation)
	{
		DxCommandList->DrawInstanced(numVerticesPerInstance, numInstances, startVertexLocation, startInstanceLocation);
	}

	void D3D12CommandList::BindDescriptorheap(RHIDescriptorHeap* descriptorHeap)
	{
		ID3D12DescriptorHeap* ppDescriptorHeaps[] = {((D3D12DescriptorHeap*)descriptorHeap)->GetDxDescriptorHeap()};
		DxCommandList->SetDescriptorHeaps(1, ppDescriptorHeaps);
	}
#pragma endregion

	/* D3D12CommandQueue */
#pragma region D3D12CommandQueue
	RHICommandList* D3D12CommandQueue::SetupNewCommandList(GraphicsAPI* api)
	{
		ID3D12CommandAllocator* dxCmdAllocator;
		D3D12CommandList* returnList;

		D3D12API* dxApi = (D3D12API*)api;

		/* Get a Command Allocator */
		/* The allocator can be reused as long as it's not 'in-flight' on the CmdQueue */
		if (!CommandAllocatorQueue.empty() && IsFenceComplete(CommandAllocatorQueue.front().FenceValue))
		{
			/* The last 'launched' allocator is in front and its fence value has been reached */
			dxCmdAllocator = CommandAllocatorQueue.front().Allocator;
			CommandAllocatorQueue.pop();

			dxCmdAllocator->Reset();
		}
		else
		{
			/* No available allocator */
			dxCmdAllocator = D3D12API::CreateCommandAllocator(dxApi->GetDxDevice(), Conversion::ToDx12(eType));
		}

		/* Get a Command List */
		if (!CommandListQueue.empty())
		{
			/* Pop off the front list */
			returnList = CommandListQueue.front();
			CommandListQueue.pop();

			((ID3D12GraphicsCommandList*)returnList->GetDxCommandList())->Reset(dxCmdAllocator, nullptr);
		}
		else
		{
			returnList = new D3D12CommandList();
			returnList->DxCommandList = D3D12API::CreateCommandList(dxApi->GetDxDevice(), dxCmdAllocator, Conversion::ToDx12(eType));
		}

		/* Set private data so we can query the allocator pointer later from the commandlist :) */
		returnList->GetDxCommandList()->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), dxCmdAllocator);

		return returnList;
	}

	void D3D12CommandQueue::ExecuteCommmandList(RHICommandList* commandList)
	{
		D3D12CommandList* d3d12CmdList = (D3D12CommandList*)commandList;
		ID3D12GraphicsCommandList* dxCmdList = d3d12CmdList->DxCommandList;

		dxCmdList->Close();

		/* Get the private held Command Allocator */
		ID3D12CommandAllocator* cmdAllocator;
		UINT dataSize = sizeof(cmdAllocator);
		dxCmdList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &cmdAllocator);

		/* Execute */
		ID3D12CommandList* const ppCommandLists[] = { dxCmdList };
		DxCommandQueue->ExecuteCommandLists(1, ppCommandLists);

		/* Signal() returns the value we will be waiting for, when the GPU is finished executing this cmdlist,
			the Fence-event will trigger... */
		UINT64 fenceValue = D3D12API::Signal(DxCommandQueue, DxFence, CurrentFenceValue);

		/* Update queues */
		CommandAllocatorQueue.emplace(CommandAllocatorEntry{ fenceValue, cmdAllocator });
		CommandListQueue.push(d3d12CmdList);

		/* Release the temporary retrieved allocator pointer */
		D3D12API::SafeRelease(cmdAllocator);
	}

	void D3D12CommandQueue::Flush()
	{
		D3D12API::FlushCommandQueue(DxCommandQueue, DxFence, CurrentFenceValue, FenceEventHandle);
	}

	D3D12CommandQueue::~D3D12CommandQueue()
	{
		Flush();

		while (!CommandAllocatorQueue.empty())
		{
			D3D12API::SafeRelease(CommandAllocatorQueue.front().Allocator);
			CommandAllocatorQueue.pop();
		}

		while (!CommandListQueue.empty())
		{
			D3D12API::SafeRelease(CommandListQueue.front()->DxCommandList);
			CommandListQueue.pop();
		}

		D3D12API::SafeRelease(DxCommandQueue);
		D3D12API::SafeRelease(DxFence);
	}

	bool D3D12CommandQueue::IsFenceComplete(UINT64 completeValue) const
	{
		return DxFence->GetCompletedValue() >= completeValue;
	}
#pragma endregion
	
	/* D3D12SwapChain */
#pragma region D3D12SwapChain
	void D3D12SwapChain::Present(RHICommandQueue* commandQueue, bool VSync)
	{
		UINT syncIntv = VSync ? 1 : 0;
		UINT flags = bIsTearingSupported && !VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
		DxgiSwapChain->Present(syncIntv, 0);

		CurrentBackBufferIndex = DxgiSwapChain->GetCurrentBackBufferIndex();
	}

	void D3D12SwapChain::Resize(GraphicsAPI* api, RHICommandQueue* commandQueue, UINT newSizeX, UINT newSizeY)
	{
		commandQueue->Flush();
	}

	D3D12SwapChain::~D3D12SwapChain()
	{
		D3D12API::SafeRelease(DxgiSwapChain);
		D3D12API::SafeRelease(DxRenderTargetDescriptorHeap);
	}
#pragma endregion

	/* D3D12DescriptorHeap */
#pragma region D3D12DescriptorHeap
	D3D12DescriptorHeap::~D3D12DescriptorHeap()
	{
		D3D12API::SafeRelease(DxDescriptorHeap);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::GetDescriptorHandle(size_t slot)
	{
		if (!IsSlotFree(slot))
		{
			// Slot is in the Freelist. Thus there's no descriptor here...
			return NullDescriptorHandle;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE handle = DxDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += (slot * DescriptorStride);

		return handle;
	}

	ID3D12DescriptorHeap* D3D12DescriptorHeap::GetDxDescriptorHeap() const
	{
		return DxDescriptorHeap;
	}

	bool D3D12DescriptorHeap::IsSlotFree(size_t slot) const
	{
		return std::find(OccupiedSlotIndices.cbegin(), OccupiedSlotIndices.cend(), slot) == OccupiedSlotIndices.cend();
	}

	size_t D3D12DescriptorHeap::GetFirstFreeSlot() const
	{
		for (int i = 0; i < NumDescriptors; ++i)
		{
			if (IsSlotFree(i)) return i;
		}

		assert(false); // no free slots?
		return std::numeric_limits<size_t>::max();
	}
#pragma endregion
}
