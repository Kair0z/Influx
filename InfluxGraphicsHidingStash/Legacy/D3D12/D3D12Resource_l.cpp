
#include "D3D12Resource.h"
#include "D3D12Conversion.h"

namespace Influx::Graphics
{
	/* API Creation Functions */
	RHIVertexBuffer* D3D12API::CreateVertexBuffer(float* initialData, UINT initialSizeInBytes, UINT strideInBytes) const
	{
		D3D12Resource* d3d12Resource = new D3D12Resource();
		D3D12VertexBuffer* d3d12Buffer = new D3D12VertexBuffer(d3d12Resource);

		if (initialData != nullptr && initialSizeInBytes > 0)
		{
			D3D12_HEAP_PROPERTIES heapProps{};
			heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
			heapProps.CreationNodeMask = 1;
			heapProps.VisibleNodeMask = 1;

			// Buffer Resource Desc.
			D3D12_RESOURCE_DESC resourceDesc{};
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			resourceDesc.Alignment = 0;
			resourceDesc.Width = initialSizeInBytes;
			resourceDesc.Height = 1;
			resourceDesc.DepthOrArraySize = 1;
			resourceDesc.MipLevels = 1;
			resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
			resourceDesc.SampleDesc.Count = 1;
			resourceDesc.SampleDesc.Quality = 0;
			resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			// Create Buffer Resource
			DxDevice->CreateCommittedResource(&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&d3d12Resource->DxResource));

			ID3D12Resource* dxResource = d3d12Resource->DxResource;

			// Copy triangle data to vxBuffer
			UINT8* pDataBegin;
			D3D12_RANGE cpuReadRange;
			cpuReadRange.Begin = 0;
			cpuReadRange.End = 0;

			dxResource->Map(0, &cpuReadRange, reinterpret_cast<void**>(&pDataBegin));
			memcpy(pDataBegin, initialData, initialSizeInBytes);
			dxResource->Unmap(0, nullptr);

			// Initialize VertexBufferView:
			d3d12Buffer->DxVertexBufferView.BufferLocation = dxResource->GetGPUVirtualAddress();
			d3d12Buffer->DxVertexBufferView.StrideInBytes = strideInBytes;
			d3d12Buffer->DxVertexBufferView.SizeInBytes = initialSizeInBytes;
		}

		return d3d12Buffer;
	}

	RHIConstantBuffer* D3D12API::CreateConstantBuffer(float* initialData, UINT initialSizeInBytes, UINT initialStrideInBytes) const
	{
		D3D12Resource* d3d12Resource = new D3D12Resource();
		D3D12ConstantBuffer* d3d12Buffer = new D3D12ConstantBuffer(d3d12Resource);

		if (initialData != nullptr && initialSizeInBytes > 0)
		{
			D3D12_HEAP_PROPERTIES heapProps{};
			heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
			heapProps.CreationNodeMask = 1;
			heapProps.VisibleNodeMask = 1;

			// Buffer Resource Desc.
			D3D12_RESOURCE_DESC resourceDesc{};
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			resourceDesc.Alignment = 0;
			resourceDesc.Width = initialSizeInBytes;
			resourceDesc.Height = 1;
			resourceDesc.DepthOrArraySize = 1;
			resourceDesc.MipLevels = 1;
			resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
			resourceDesc.SampleDesc.Count = 1;
			resourceDesc.SampleDesc.Quality = 0;
			resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			// Create Buffer Resource
			DxDevice->CreateCommittedResource(&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&d3d12Resource->DxResource));

			ID3D12Resource* dxResource = d3d12Resource->DxResource;

			// Describe and create a constant buffer view.
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = dxResource->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = initialSizeInBytes;
			DxDevice->CreateConstantBufferView(&cbvDesc,
				ResourceDescriptorHeap->DxDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

			// Copy triangle data to vxBuffer
			UINT8* pDataBegin;
			D3D12_RANGE cpuReadRange;
			cpuReadRange.Begin = 0;
			cpuReadRange.End = 0;

			dxResource->Map(0, &cpuReadRange, reinterpret_cast<void**>(&pDataBegin));
			memcpy(pDataBegin, initialData, initialSizeInBytes);
			dxResource->Unmap(0, nullptr);
		}

		return d3d12Buffer;
	}

	RHITexture* D3D12API::CreateTexture(const RHITextureDescription& constructionArgs) const
	{
		D3D12Texture* d3d12Texture = new D3D12Texture();
		d3d12Texture->ConstructionDescription = constructionArgs;

		// Create Buffer Resource
		D3D12Resource* d3d12Resource = new D3D12Resource();
		d3d12Texture->Resource = d3d12Resource;
		d3d12Resource->CurrentState = d3d12Resource->PreviousState = constructionArgs.InitialResourceState;

		D3D12_HEAP_PROPERTIES heapProps{};
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Alignment = 0;
		resourceDesc.Width	= static_cast<UINT64>(constructionArgs.Width);
		resourceDesc.Height = static_cast<UINT64>(constructionArgs.Height);
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = constructionArgs.MipLevels;
		resourceDesc.Format = Conversion::ToDx12(constructionArgs.Format);
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		D3D12_CLEAR_VALUE optimizedClearValue{};
		FLOAT clearColor[4]
			= { constructionArgs.OptimizedClearValue[0], constructionArgs.OptimizedClearValue[1],
			constructionArgs.OptimizedClearValue[2], constructionArgs.OptimizedClearValue[3] };

		optimizedClearValue.Format = resourceDesc.Format;
		memcpy(optimizedClearValue.Color, &constructionArgs.OptimizedClearValue, sizeof(optimizedClearValue.Color));

		DxDevice->CreateCommittedResource(&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			Conversion::ToDx12(constructionArgs.InitialResourceState),
			&optimizedClearValue,
			IID_PPV_ARGS(&d3d12Resource->DxResource));

		// Todo: Upload Heap?

		// Create RTV:
		d3d12Texture->RenderTargetView = D3D12API::CreateRenderTargetView(d3d12Texture);

		return d3d12Texture;
	}

	RHIRenderTargetView* D3D12API::CreateRenderTargetView(RHITexture* texture) const
	{
		// Find first free index slot
		size_t freeIndex = RTVDescriptorHeap->GetFirstFreeSlot();
		RTVDescriptorHeap->OccupiedSlotIndices.push_back(freeIndex);

		D3D12_CPU_DESCRIPTOR_HANDLE cpu_desc_handle = RTVDescriptorHeap->DxDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		cpu_desc_handle.ptr += (GetRTVDescriptorSize() * freeIndex);
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_desc_handle = RTVDescriptorHeap->DxDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		gpu_desc_handle.ptr += (GetRTVDescriptorSize() * freeIndex);

		D3D12RenderTargetView* d3d12RTV = new D3D12RenderTargetView();

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = Conversion::ToDx12(texture->GetRHIFormat());
		rtvDesc.Texture2D.MipSlice = 0;
		rtvDesc.Texture2D.PlaneSlice = 0;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		D3D12Resource* d3d12Resource = (D3D12Resource*)((D3D12Texture*)texture)->Resource;
		DxDevice->CreateRenderTargetView(d3d12Resource->DxResource,
			&rtvDesc, cpu_desc_handle);

		d3d12RTV->DxCPUHandle = cpu_desc_handle;
		d3d12RTV->DxGPUHandle = gpu_desc_handle;

		return d3d12RTV;
	}

	/* D3D12Texture */
	D3D12Texture::D3D12Texture() : RHITexture() {}

	/* D3D12Resource */
	D3D12Resource::D3D12Resource() : D3D12Resource(nullptr, ERHIResourceState::Invalid) {}

	D3D12Resource::D3D12Resource(ID3D12Resource* dxResource, ERHIResourceState initialState) : RHIResource()
	{
		PreviousState = CurrentState = initialState;
		DxResource = dxResource;
	}

	D3D12Resource::~D3D12Resource()
	{
		D3D12API::SafeRelease(DxResource);
	}

	ID3D12Resource* D3D12Resource::GetDxResource() const
	{
		return DxResource;
	}

	D3D12ConstantBuffer::D3D12ConstantBuffer(D3D12Resource* gpuResource)
	{
	}

	D3D12VertexBuffer::D3D12VertexBuffer(D3D12Resource* gpuResource)
	{
		GpuResource = gpuResource;
	}

	D3D12_VERTEX_BUFFER_VIEW D3D12VertexBuffer::GetDxVertexBufferView() const
	{
		return DxVertexBufferView;
	}
}