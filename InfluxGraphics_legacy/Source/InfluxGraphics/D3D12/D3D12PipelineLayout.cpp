#include "InfluxGraphics/D3D12/D3D12PipelineLayout.h"

namespace Influx::Graphics
{
	ID3D12RootSignature* D3D12GraphicsPipelineLayout::GetDxRootSignature() const
	{
		return mp_dxRootSignature;
	}
}

