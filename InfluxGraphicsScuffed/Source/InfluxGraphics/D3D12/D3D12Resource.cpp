#include "InfluxGraphics/Common.h"
#include "InfluxGraphics/D3D12/D3D12Resource.h"

namespace Influx::Graphics
{
	void D3D12Resource::OnTransitionState(const ERHIResourceState before, const ERHIResourceState after)
	{

	}

	ID3D12Resource* D3D12Resource::GetDxResource() const
	{
		return mp_dxResource;
	}

	D3D12Resource::~D3D12Resource()
	{
		D3D12::SafeRelease(mp_dxResource);
	}
}