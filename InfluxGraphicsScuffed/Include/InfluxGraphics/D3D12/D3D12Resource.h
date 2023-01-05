#pragma once

#ifndef __GR_D3D12_RESOURCE_H_
#define __GR_D3D12_RESOURCE_H_

#include "InfluxGraphics/RHIResource.h"
#include "D3D12.h"

namespace Influx::Graphics
{
	class D3D12Resource final : public RHIResource
	{
		friend class D3D12Device;	// Only the device can create these...
		D3D12Resource(const ERHIResourceState initialState) : RHIResource(initialState) {}

		virtual void OnTransitionState(const ERHIResourceState before, const ERHIResourceState after) override final;

		ID3D12Resource* mp_dxResource;

	public:
		ID3D12Resource* GetDxResource() const;

		virtual ~D3D12Resource();
	};

	class D3D12RenderTargetView final : public RHIRenderTargetView
	{
		friend class D3D12Device;
		D3D12RenderTargetView() : RHIRenderTargetView() {}

		D3D12_CPU_DESCRIPTOR_HANDLE m_dxCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_dxGpuHandle;

	public:
		virtual ~D3D12RenderTargetView() = default;
	};
}

#endif

