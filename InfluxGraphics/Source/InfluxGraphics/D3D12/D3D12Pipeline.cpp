#include "InfluxGraphics/D3D12/D3D12Pipeline.h"

namespace Influx::Graphics
{
	ID3D12PipelineState* D3D12GraphicsPipeline::GetDxPipelineState() const
	{
		return mp_dxPipelineState;
	}
}


