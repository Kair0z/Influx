#pragma once

#include "Runtime/RHI/SwapChain.h"
#include "D3D12API.h"

namespace Influx
{
#pragma region ForwardDeclarations
	class D3D12API;
	class D3D12CommandQueue;
#pragma endregion

	class D3D12SwapChain final : public RHISwapChain
	{
	public:
		static Ptr<D3D12SwapChain> Create(const Ptr<D3D12API> api, const SwapChainDesc& desc, Ptr<ID3D12CommandQueue> cmdQueue);

		virtual void Present(const PresentDescription& presentDesc) override final;
		virtual Ptr<RHIRenderTarget> GetCurrentRenderTarget() const override final;
		virtual Ptr<RHIRenderTarget> GetDepthTarget() const override final;
		virtual void Resize(const Ptr<RenderAPI> api, Ptr<RHICommandQueue> cmdQueue, const Vector2u& newSize) override final;

		Ptr<ID3D12Resource> GetCurrentBackBufferResource() const;
		Ptr<ID3D12Resource> GetDepthBufferResource() const;

		D3D12SwapChain(const D3D12SwapChain&) = delete;
		D3D12SwapChain(D3D12SwapChain&&) = delete;
		D3D12SwapChain& operator=(const D3D12SwapChain&) = delete;
		D3D12SwapChain& operator=(D3D12SwapChain&&) = delete;
		~D3D12SwapChain();

	private:
		D3D12SwapChain(const SwapChainDesc& desc) : RHISwapChain(desc) {}

		Ptr<IDXGISwapChain4> DxSwapChain{};
		Ptr<ID3D12Resource> BackBufferResources[StatNumBackBuffers]{};
		Ptr<ID3D12Resource> DepthBufferResource{};
	};
}


