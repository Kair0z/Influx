#pragma once

#ifndef __GR_RHI_PIPELINE_H_
#define __GR_RHI_PIPELINE_H_

#include "Types.h"

namespace Influx::Graphics
{
	struct RHIPipelineDescription final
	{
		using ShaderCodePtr = void*;

		RHIRasterizerState RasterizerState;
		RHIBlendState BlendState;
		RHIDepthStencilState DepthStencilState;

		ERHIPrimitiveTopologyType PrimitiveTopologyType;

		ShaderCodePtr VS;
		ShaderCodePtr PS;
		ShaderCodePtr DS;
		ShaderCodePtr HS;
		ShaderCodePtr GS;

		uint8 SampleCount;
		uint8 SampleQuality;
		uint8 SampleMask;
		uint8 NodeMask;

		// Input Elements...

		struct
		{
			ERHIFormat Format;

			bool bEnableBlend;
			bool bEnableLogicOp;
		} RenderTargets[8];
	};

	/* RHIPipeline */
	class RHIPipeline
	{
	protected:
		RHIPipeline(const RHIPipelineDescription& desc);

	public:
		RHIPipeline(const RHIPipeline&) = delete;
		RHIPipeline(RHIPipeline&&) = delete;
		RHIPipeline& operator=(const RHIPipeline&) = delete;
		RHIPipeline& operator=(RHIPipeline&&) = delete;
		virtual ~RHIPipeline() = default;

	private:
		RHIPipelineDescription m_pipelineDescription;
	};
}

#endif