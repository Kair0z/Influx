#pragma once
#include "influx_graphics/pipeline.h"
#include "influx_graphics/d3d12/dx12_base.h"

struct ID3D12PipelineState;

namespace influx::graphics
{
	class dx12_pipeline final
		: public pipeline
		, public dx12_base
	{
	public:
		dx12_pipeline(ID3D12PipelineState* dxpipeline, const pipeline_desc& desc);

	private:
		ID3D12PipelineState* mpdx_pipeline;
	};
}