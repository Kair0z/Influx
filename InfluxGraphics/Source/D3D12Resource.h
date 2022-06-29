#pragma once

#include "RHIResource.h"
#include "D3D12API.h"

namespace Influx::Graphics
{
	class D3D12Resource final : public RHIResource
	{
	public:
		D3D12Resource();
		D3D12Resource(ID3D12Resource* dxResource, ERHIResourceState initialState);
		ID3D12Resource* GetDxResource() const;

		virtual ~D3D12Resource();

	private:
		ID3D12Resource* DxResource;
		friend class D3D12API;
	};

	class D3D12Texture final : public RHITexture
	{
	public:
		~D3D12Texture() = default;

	private:
		ID3D12DescriptorHeap* DxDescriptorHeap;
		int DescriptorHeapIndex;

		D3D12Texture() = default;
		friend class D3D12API;
	};

	class D3D12ConstantBufferView final : public RHIConstantBufferView
	{
		D3D12ConstantBufferView() = default;
		D3D12_CPU_DESCRIPTOR_HANDLE DxCPUHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE DxGPUHandle;
	};

	class D3D12RenderTargetView final : public RHIRenderTargetView
	{
	public:
		D3D12RenderTargetView() = default;
		D3D12_CPU_DESCRIPTOR_HANDLE DxCPUHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE DxGPUHandle;
	};

	class D3D12VertexBuffer final : public RHIVertexBuffer
	{
	public:
		D3D12VertexBuffer(D3D12Resource* gpuResource);
		D3D12_VERTEX_BUFFER_VIEW GetDxVertexBufferView() const;

		virtual ~D3D12VertexBuffer() = default;

	private:
		D3D12_VERTEX_BUFFER_VIEW DxVertexBufferView;

		D3D12VertexBuffer() = default;
		friend class D3D12API;
	};

	class D3D12ConstantBuffer final : public RHIConstantBuffer
	{
	public:
		D3D12ConstantBuffer(D3D12Resource* gpuResource);

	private:
		D3D12ConstantBuffer() = default;
		friend class D3D12API;
	};
}