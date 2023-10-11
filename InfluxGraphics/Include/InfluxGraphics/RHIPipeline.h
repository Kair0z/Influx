#pragma once

#ifndef __GR_RHI_PIPELINE_H_
#define __GR_RHI_PIPELINE_H_

#include "Types.h"

#include "Core/Container/Containers.h"
#include "Core/Container/Map.h"

namespace influx::Graphics
{
	struct RHIGraphicsPipelineDescription final
	{
		using CompiledShaderData = vector<uint8>;

		struct InputElement final
		{
			InputElement(const string& name, uint8 semanticIndex, ERHIFormat format, uint8 inputSlot, uint8 alignedByteOffset, bool dataPerVertexNotPerInstance, uint8 instanceDataStepRate)
				: SemanticName{ name }, SemanticIndex{ semanticIndex }, Format{ format }, InputSlot{ inputSlot }, AlignedByteOffset{ alignedByteOffset }
				, bDataPerVertexNotPerInstance{ dataPerVertexNotPerInstance }, InstanceDataStepRate{ instanceDataStepRate } {}

			string SemanticName;
			uint8 SemanticIndex;
			ERHIFormat Format;
			uint8 InputSlot;
			uint8 AlignedByteOffset;
			bool bDataPerVertexNotPerInstance;
			uint8 InstanceDataStepRate;
		};
		
		vector<InputElement> InputElements;

		RHIRasterizerState RasterizerState;
		RHIBlendState BlendState;
		RHIDepthStencilState DepthStencilState;

		ERHIPrimitiveTopologyType PrimitiveTopologyType;

		CompiledShaderData VS;
		CompiledShaderData PS;
		CompiledShaderData DS;
		CompiledShaderData HS;
		CompiledShaderData GS;

		uint8 SampleCount		= 1u;
		uint8 SampleQuality		= 0u;
		uint8 SampleMask		= 255u;
		uint8 NodeMask			= 0u;

		struct
		{
			ERHIFormat Format = ERHIFormat::INVALID;

			struct
			{
				bool bEnableBlend = false;
				bool bEnableLogicOp = false;

				ERHIBlend SrcBlend						= ERHIBlend::One;
				ERHIBlend DestBlend						= ERHIBlend::Zero;
				ERHIBlendOperation BlendOperation		= ERHIBlendOperation::OpAdd;

				ERHIBlend SrcBlendAlpha					= ERHIBlend::One;
				ERHIBlend DestBlendAlpha				= ERHIBlend::Zero;
				ERHIBlendOperation BlendOperationAlpha	= ERHIBlendOperation::OpAdd;

				ERHILogicOperation LogicOperation = ERHILogicOperation::NoOp;

				uint8 RenderTargetWriteMask = 15u;

			} BlendDesc;

		} RenderTargets[8];

		// Making this comparable
		bool operator==(const RHIGraphicsPipelineDescription& other) const
		{
			return SampleCount == other.SampleCount;
		}
	};

	/* RHIGraphicsPipeline */
	class RHIGraphicsPipeline
	{
	protected:
		RHIGraphicsPipeline(const RHIGraphicsPipelineDescription& desc);

	public:
		RHIGraphicsPipeline(const RHIGraphicsPipeline&) = delete;
		RHIGraphicsPipeline(RHIGraphicsPipeline&&) = delete;
		RHIGraphicsPipeline& operator=(const RHIGraphicsPipeline&) = delete;
		RHIGraphicsPipeline& operator=(RHIGraphicsPipeline&&) = delete;
		virtual ~RHIGraphicsPipeline() = default;

	private:
		RHIGraphicsPipelineDescription m_pipelineDescription;
	};
}

// Define Hash function:
namespace std
{
	template <>
	struct std::hash<influx::Graphics::RHIGraphicsPipelineDescription>
	{
		std::size_t operator()(const influx::Graphics::RHIGraphicsPipelineDescription& desc) const noexcept
		{
			return 0u;
		}
	};
}
#endif