#pragma once
#include "influx_graphics/pipeline.h"

struct ID3D12PipelineState;

namespace influx::graphics
{
	class dx12_pipeline final : public pipeline
	{
		ID3D12PipelineState* mpdx_pipeline;

	private:
		dx12_pipeline(ID3D12PipelineState* dxpipeline, const pipeline_desc& desc);
		virtual void release_impl(device*) override;
		friend class dx12_device;
	};
}