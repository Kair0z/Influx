#include "InfluxGraphics/Common.h"
#include "InfluxGraphics/D3D12/D3D12CommandList.h"

namespace Influx::Graphics
{
	D3D12CommandList::D3D12CommandList(const ERHICommandQueueType type)
		: RHICommandList(type)
	{

	}

	D3D12CommandList::~D3D12CommandList()
	{
		// ...
	}

	ID3D12GraphicsCommandList* D3D12CommandList::GetDxCommandList() const
	{
		return mp_dxCommandList;
	}
}

