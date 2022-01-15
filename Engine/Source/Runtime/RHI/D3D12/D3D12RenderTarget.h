#pragma once

#include "Runtime/RHI/RenderTarget.h"
#include "D3D12API.h"

#include "Core/Memory/Reference.h"
#include "Core/Container/Containers.h"

namespace Influx
{
	class D3D12RenderTarget final : public RHIRenderTarget
	{
	public:
		static Ptr<D3D12RenderTarget> Create(const Ptr<D3D12API> api, const Vector2u& dimensions, const ERHIFormat format, const RHIRenderTarget::ERenderTargetType type, const RenderTargetConfig& config = RenderTargetConfig());
		static Ptr<D3D12RenderTarget> CreateDepthStencil(const Ptr<D3D12API> api, const Vector2u& dimensions, const ERHIFormat format, const RenderTargetConfig& config = RenderTargetConfig());
		static Ptr<D3D12RenderTarget> CreateRenderTarget(const Ptr<D3D12API> api, const Vector2u& dimensions, const ERHIFormat format, const RenderTargetConfig& config = RenderTargetConfig());
		
		static Ptr<D3D12RenderTarget> CreateFromResource(const Ptr<D3D12API> api, const Vector2u& dimensions, const ERHIFormat format, const RHIRenderTarget::ERenderTargetType type, Ptr<ID3D12Resource> bufferResource, const RenderTargetConfig& config = RenderTargetConfig());

		/* Resize & recreate resources */
		virtual void Resize(const Ptr<RenderAPI> api, const Vector2u& newSize) override final;

		Ptr<ID3D12DescriptorHeap> GetDescriptorHeap() const;
		ID3D12Resource* GetBufferResource() const;

		D3D12_CPU_DESCRIPTOR_HANDLE GetViewCPUHandle() const;
		D3D12_GPU_DESCRIPTOR_HANDLE GetViewGPUHandle() const;
		void UpdateView(ID3D12Device2* device);

		virtual ~D3D12RenderTarget();

	private:
		ID3D12DescriptorHeap* mpDescriptorHeap;
		ID3D12Resource* mpBufferResource;
		CD3DX12_CPU_DESCRIPTOR_HANDLE mViewHandle{};

		void CreateBufferResource(ID3D12Device2* device);

		D3D12RenderTarget(const Vector2u& dimensions, const ERHIFormat format, 
			const RHIRenderTarget::ERenderTargetType type, const RenderTargetConfig& config = RenderTargetConfig())
			: RHIRenderTarget(dimensions, format, type, config) {}

	public:
		D3D12RenderTarget(const D3D12RenderTarget&) = delete;
		D3D12RenderTarget(D3D12RenderTarget&&) = delete;
		D3D12RenderTarget& operator=(const D3D12RenderTarget&) = delete;
		D3D12RenderTarget& operator=(D3D12RenderTarget&&) = delete;
	};
}


