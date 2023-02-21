#pragma once

#ifndef __GR_RHI_TYPES_H_
#define __GR_RHI_TYPES_H_

#include "Core/BasicTypes.h"
#include "Core/Math/Vector.h"

namespace Influx::Graphics
{
	/* Graphics APIs */
	enum class EGraphicsAPI
	{
		D3D12,
		Max,
		NotSupported = Max
	};

	// Types of resources bindable to a Pipeline
	enum class ERHIResourceBindingType
	{
		Constants,
		CBV,
		SRV,
		UAV
	};

	enum class ERHIResourceFlags
	{
		None = 0,
		AllowRenderTarget = 0x1,
		AllowDepthStencil = 0x2,
		AllowUnorderedAccess = 0x4,
		DenyShaderResource = 0x8,
		AllowCrossAdapter = 0x10,
		AllowSimultaneousAccess = 0x20,
		VideoDecodeReferenceOnly = 0x40,
		VideoEncodeReferenceOnly = 0x80
	};

	enum class ERHIResourceState
	{
		Common,
		VertexAndConstantBuffer,
		IndexBuffer,
		RenderTarget,
		UnorderedAccess,
		DepthWrite,
		DepthRead,
		Present,
		RaytracingAS,
		CopyDest,
		CopySource,
		GenericRead,
		AllShaderResource,
		NonPixelReadResource,
		PixelShaderResource,
		Undefined,
		Invalid
	};

	enum class ERHIResourceViewType
	{
		Resource,
		CBV = Resource,
		UAV = Resource,
		SRV = Resource,
		DSV,
		RTV,
		Sampler,
		Max,
		Invalid = Max
	};
	using ERHIDescriptorType = ERHIResourceViewType;

	enum class ERHIShaderType
	{
		VertexShader,
		PixelShader
	};

	enum class ERHIShaderStageFlags
	{
		Default
	};

	enum class ERHIFormat
	{
		/* 4 */
		RGBA_32_Float,
		RGBA_8_Unorm,

		/* 3 */
		RGB_32_Float,

		/* 1 */
		R_16_Uint,
		D_32_Float,

		INVALID
	};

	enum class ERHISampleCount
	{
		_1,
		_2,
		_4,
		_8,
		_16,
		_32,
		_64
	};

	enum class ERHIPrimitiveTopology
	{
		TriangleList
	};

	enum class ERHIPrimitiveTopologyType
	{
		Triangle
	};

	enum class ERHICommandQueueType
	{
		Graphics,
		Compute,
		Max
	};

	enum class ERHICullMode
	{
		None,
		BackFaceCull,
		FrontFaceCull
	};

	enum class ERHIFillMode
	{
		Solid,
		Wireframe
	};

	enum class ERHIPipelineBindPoint
	{
		Graphics
	};

	enum class ERHIShaderModel
	{
		SM_5_0
	};

	enum class ERHIComparisonFunc
	{
		Never = 1,
		Less = 2,
		Equal = 3,
		LessEq = 4,
		Greater = 5,
		NotEqual = 6,
		GreaterEq = 7,
		Always = 8
	};

	// RenderPass
	enum class ERHIRenderPassAttachmentType
	{
		Color
	};

	enum class ERHIRenderPassLoadOp
	{
		Load,			// Preserve the existing contents of the attachment
		Clear,			// Clear the values to a constant at the start
		DontCare		// Existing contents are undefined; we don't care about them
	};

	enum class ERHIRenderPassStoreOp
	{
		Store,			// Rendered contents will be stored in memory and can be read later
		DontCare		// Contents of the framebuffer will be undefined after the rendering operation
	};

	struct RHIClearValue final
	{
		static RHIClearValue Default()
		{
			RHIClearValue result{};
			result.Colour = {};
			return result;
		}

		Math::Vectorf4 Colour;
	};

	/* Viewport */
	struct RHIViewport final
	{
		RHIViewport() = default;
		RHIViewport(float width, float height, float left = 0.0f, float bottom = 0.0f)
			: Width{ width }, Height{ height }, Left{ left }, Bottom{ bottom } {}

		float Width;
		float Height;
		float Bottom;
		float Left;
	};

	/* ScissorRect */
	struct RHIScissorRect final
	{
		RHIScissorRect() = default;
		RHIScissorRect(uint32 width, uint32 height, uint32 left = 0.0f, uint32 bottom = 0.0f)
			: Width{ width }, Height{ height }, Left{ left }, Bottom{ bottom } {}

		uint32 Width;
		uint32 Height;
		uint32 Bottom;
		uint32 Left;
	};

	// Pipeline:
	/* Pipeline: Rasterizer State */
	struct RHIRasterizerState final
	{
		constexpr static RHIRasterizerState GetDefault()
		{
			RHIRasterizerState def{};
			def.Fillmode = ERHIFillMode::Solid;
			def.Cullmode = ERHICullMode::BackFaceCull;
			def.bFrontCounterClockwise = false;
			def.DepthBias = 0.0f;
			def.DepthBiasClamp = 0.0f;
			def.SlopeScaledDepthBias = 0.0f;
			def.bEnableDepthClip = true;
			def.bEnableMultisample = false;
			def.bEnableLineAA = false;
			def.ForcedSampleCount = 0u;
			def.bEnableConservativeRaster = false;
			return def;
		}

		ERHIFillMode Fillmode;
		ERHICullMode Cullmode;
		bool bFrontCounterClockwise;
		int DepthBias;
		float DepthBiasClamp;
		float SlopeScaledDepthBias;
		bool bEnableDepthClip;
		bool bEnableMultisample;
		bool bEnableLineAA;
		bool bEnableConservativeRaster;
		uint8 ForcedSampleCount;
	};

	/* Pipeline: Depth & Stencil State */
	struct RHIDepthStencilState final
	{
		constexpr static RHIDepthStencilState GetDefault()
		{
			RHIDepthStencilState def{};
			def.bEnableDepth = false;
			def.bEnableStencil = false;
			return def;
		}

		bool bEnableDepth;
		bool bEnableStencil;
		ERHIComparisonFunc DepthFunc;
		
		uint8 StencilReadMask;
		uint8 StencilWriteMask;
	};

	struct RHIBlendState final
	{
		bool bEnableAlphaToCoverage		= false;
		bool bEnableIndependentBlend	= false;
	};
}

#endif