#pragma once
#include "influx_graphics/pipeline.h"

struct ID3D12PipelineState;

namespace influx::graphics
{
	template <e_pipeline_type _t>
	class dx12_pipeline final : public pipeline<_t>
	{
		ID3D12PipelineState* mpdx_pipeline;
		ID3D12StateObject* mpdx_raytracing_state_object; // I hate Dx12

		dx12_pipeline(ID3D12StateObject* rtdxpipeline, const pipeline_desc<_t>& desc)
			: pipeline<_t>(desc)
		{
			static_assert(_t == e_pipeline_type::raytracing);
			base::mp_native = mpdx_raytracing_state_object = rtdxpipeline;
			mpdx_pipeline = nullptr;
		}

		dx12_pipeline(ID3D12PipelineState* dxpipeline, const pipeline_desc<_t>& desc)
			: pipeline<_t>(desc)
		{
			base::mp_native = mpdx_pipeline = dxpipeline;
			mpdx_raytracing_state_object = nullptr;
		}

		virtual void release_impl(device*) override
		{
			mpdx_pipeline->Release();
		}

		friend class dx12_device;
	};
}