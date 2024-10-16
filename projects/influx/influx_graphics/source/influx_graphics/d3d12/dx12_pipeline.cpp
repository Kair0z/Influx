#include "graphics_pch.h"
#include "dx12_pipeline.h"

#include "dx12_headers.h"

namespace influx::graphics
{
	dx12_pipeline::dx12_pipeline(ID3D12PipelineState* dxpipeline, const pipeline_desc& desc)
		: pipeline(desc)
	{
		mp_native = mpdx_pipeline = dxpipeline;
		set_releasable(mpdx_pipeline);
	}
}

