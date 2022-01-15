#pragma once
#include "PipelineBuilder.h"

namespace Influx
{
	class RHIGraphicsPipeline
	{
	public:
		RHIGraphicsPipeline(const RHIGraphicsPipeline&) = default;
		RHIGraphicsPipeline(RHIGraphicsPipeline&&) = default;
		RHIGraphicsPipeline& operator=(const RHIGraphicsPipeline&) = default;
		RHIGraphicsPipeline& operator=(RHIGraphicsPipeline&&) = default;

		virtual ~RHIGraphicsPipeline() = default;

	protected:
		inline RHIGraphicsPipeline(const GraphicsPipelineBuilder& PipelineLayout) : OriginalPipelineBuilder{ PipelineLayout } {};

		const GraphicsPipelineBuilder& OriginalPipelineBuilder{};
	};
}


