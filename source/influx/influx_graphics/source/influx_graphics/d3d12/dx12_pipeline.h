#pragma once
#include "influx_graphics/pipeline.h"

struct ID3D12PipelineState;

namespace influx::graphics
{
	template <e_pipeline_type _t>
	class dx12_pipeline final : public pipeline<_t>
	{
		ID3D12PipelineState* mpdx_pipeline;

		dx12_pipeline(ID3D12PipelineState* dxpipeline, const pipeline_desc<_t>& desc)
			: pipeline<_t>(desc)
		{
			base::mp_native = mpdx_pipeline = dxpipeline;
		}

		virtual void release_impl(device*) override
		{
			mpdx_pipeline->Release();
		}

		friend class dx12_device;
	};
}