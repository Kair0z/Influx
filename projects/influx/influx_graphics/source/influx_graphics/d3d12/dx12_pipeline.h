#pragma once
#include "influx_graphics/pipeline.h"

struct ID3D12PipelineState;

namespace influx::graphics
{
	class dx12_pipeline final : public pipeline
	{
	public:
		dx12_pipeline(ID3D12PipelineState* dxpipeline, const pipeline_desc& desc);

	private:
		ID3D12PipelineState* mpdx_pipeline;
	};
}