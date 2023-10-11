#pragma once

#ifndef __GR_RHI_PIPELINELAYOUT_H_
#define __GR_RHI_PIPELINELAYOUT_H_

#include "Types.h"

#include "Core/Container/Map.h"

namespace influx::Graphics
{
	struct RHIGraphicsPipelineLayoutDescription final
	{
		uint32 ID = 0u;

		// Making this comparable
		bool operator==(const RHIGraphicsPipelineLayoutDescription& other) const
		{
			return ID == other.ID;
		}
	};

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

// Define Hash function:
namespace std
{
	template <>
	struct std::hash<influx::Graphics::RHIGraphicsPipelineLayoutDescription>
	{
		std::size_t operator()(const influx::Graphics::RHIGraphicsPipelineLayoutDescription& desc) const noexcept
		{
			return desc.ID;
		}
	};
}
#endif