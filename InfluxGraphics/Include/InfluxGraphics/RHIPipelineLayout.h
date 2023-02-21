#pragma once

#ifndef __GR_RHI_PIPELINELAYOUT_H_
#define __GR_RHI_PIPELINELAYOUT_H_

#include "Types.h"

namespace Influx::Graphics
{
	/* 
	* RHIPipelineLayout
	* RHIRootSignature 
	*/
	class RHIGraphicsPipelineLayout
	{
	protected:
		RHIGraphicsPipelineLayout() = default;

	public:
		RHIGraphicsPipelineLayout(const RHIGraphicsPipelineLayout&) = delete;
		RHIGraphicsPipelineLayout(RHIGraphicsPipelineLayout&&) = delete;
		RHIGraphicsPipelineLayout& operator=(const RHIGraphicsPipelineLayout&) = delete;
		RHIGraphicsPipelineLayout& operator=(RHIGraphicsPipelineLayout&&) = delete;
		virtual ~RHIGraphicsPipelineLayout() = default;
	};
}

#endif