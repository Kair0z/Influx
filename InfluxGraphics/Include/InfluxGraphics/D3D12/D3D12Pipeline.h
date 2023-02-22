#pragma once

#ifndef __GR_D3D12_PIPELINE_H_
#define __GR_D3D12_PIPELINE_H_

#include "InfluxGraphics/RHIPipeline.h"
#include "D3D12.h"

namespace Influx::Graphics
{
	/* D3D12Pipeline */
	class D3D12GraphicsPipeline final : public RHIGraphicsPipeline
	{
		friend class D3D12Device;
		D3D12GraphicsPipeline(const RHIGraphicsPipelineDescription& desc) : RHIGraphicsPipeline(desc) {}

		ID3D12PipelineState* mp_dxPipelineState = nullptr;

	public:
		ID3D12PipelineState* GetDxPipelineState() const;
	};
}

#endif