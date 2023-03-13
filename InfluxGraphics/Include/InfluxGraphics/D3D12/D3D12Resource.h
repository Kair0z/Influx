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
		D3D12Resource(ERHIResourceState initialState, const RHIClearValue& optimizedClearValue, const uint64& numBytes)
			: RHIResource(initialState, optimizedClearValue, numBytes) {}

		virtual void* Map() const override;
		virtual void ScopedMap(Function<void(void*)> mapFunction) const override;
		virtual void Unmap() const override;

		virtual void OnTransitionState(const ERHIResourceState before, const ERHIResourceState after) override final;

		ID3D12Resource* mp_dxResource;

	public:
		ID3D12Resource* GetDxResource() const;

		virtual ~D3D12Resource();
	};
}

#endif

