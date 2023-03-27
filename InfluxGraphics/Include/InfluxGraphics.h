#pragma once

#ifndef __INFLUX_GRAPHICS_H_
#define __INFLUX_GRAPHICS_H_

// Defines
#pragma region Defines
#define INFLUX_GRAPHICS_USE_CORE		1
#define INFLUX_GRAPHICS_USE_STL			1

#define INFLUX_GRAPHICS_INCLUDE_DX12	1
#define INFLUX_GRAPHICS_INCLUDE_VULKAN	0

#define INFLUX_GRAPHICS_DEBUG			_DEBUG
#pragma endregion

// RHI types
#pragma region RHI Types

#pragma region RHI Types - Core
#if INFLUX_GRAPHICS_USE_CORE
#include "Core/BasicTypes.h"
#include "Core/Math/Vector.h"
#include "Core/Math/Matrix.h"
#include "Core/Container/Vector.h"
#include "Core/Container/Array.h"
#include "Core/String.h"
#include "Core/Function.h"
#endif

#if INFLUX_GRAPHICS_USE_STL
#include <vector>
#include <array>
#include <string>
#endif

namespace Influx::Graphics
{
#if INFLUX_GRAPHICS_USE_CORE
	using uint8		= Influx::uint8;
	using byte		= Influx::byte;
	using uint16	= Influx::uint16;
	using uint32	= Influx::uint32;
	using uint64	= Influx::uint64;

	using int8		= Influx::int8;
	using int16		= Influx::int16;
	using int32		= Influx::int32;
	using int64		= Influx::int64;

	using f32		= Influx::f32;
	using f64		= Influx::f64;

	constexpr uint64 u64_max	= Influx::u64_max;
	constexpr uint32 u32_max	= Influx::u32_max;
	constexpr uint16 u16_max	= Influx::u16_max;
	constexpr uint8  u8_max		= Influx::u8_max;

	using Vectorf2 = Influx::Math::Vectorf2;
	using Vectorf3 = Influx::Math::Vectorf3;
	using Vectorf4 = Influx::Math::Vectorf4;

	using Vectoru2 = Influx::Math::Vectoru2;
	using Vectoru3 = Influx::Math::Vectoru3;
	using Vectoru4 = Influx::Math::Vectoru4;

	using Matrix4x4f = Influx::Math::Matrix4x4f;

	template <typename _T>
	using Vector	= Influx::Vector<_T>;

	template <typename _T, uint64 _N>
	using Array		= Influx::Array<_T, _N>;

	using String = Influx::String;

	template <typename _F>
	using Function = Influx::Function<_F>;

#else
	using uint8		= unsigned char;
	using byte		= unsigned char;
	using uint16	= unsigned short;
	using uint32	= unsigned int;
	using uint64	= unsigned long long;

	using int8		= char;
	using int16		= short;
	using int32		= int;
	using int64		= long;

	using f32		= float;
	using f64		= double;

	constexpr uint64 u64_max = { 0xffff'ffff'ffff'ffffui64 };
	constexpr uint32 u32_max = { 0xffff'ffffui32 };
	constexpr uint16 u16_max = { 0xffffui16 };
	constexpr uint8  u8_max = { 0xffui8 };

#if INFLUX_GRAPHICS_USE_STL
	template <typename _T>
	using Vector = std::vector<_T>;

	template <typename _T, uint64 _N>
	using Array = std::array<_T, _N>;

	using String = std::string;
#endif
#endif
}
#pragma endregion

#pragma region RHI Types - Enum
namespace Influx::Graphics
{
	/* Graphics APIs */
	enum class EGraphicsAPI
	{
#if INFLUX_GRAPHICS_INCLUDE_DX12
		D3D12,
#endif
#if INFLUX_GRAPHICS_INCLUDE_VULKAN
		Vulkan,
#endif
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

		/* 2 */
		RG_32_Float,

		/* 1 */
		R_16_Uint,
		D_32_Float,

		Unknown,
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
		TriangleList,
		TriangleListAdj,
		TriangleStrip,
		TriangleStripAdj,
		Max
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

	enum class ERHIBlend
	{
		Zero = 1,
		One = 2,
		SrcColour = 3,
		InvSrcColour = 4,
		SrcAlpha = 5,
		InvSrcAlpha = 6,
		DestAlpha = 7,
		InvDestAlpha = 8,
		DestColour = 9,
		InvDestColour = 10,
		SrcAlphaSat = 11,
		BlendFactor = 14,
		InvBlendFactor = 15,
		Src1Colour = 16,
		InvSrc1Colour = 17,
		Src1Alpha = 18,
		InvSrc1Alpha = 19,
		Max
	};

	enum class ERHIBlendOperation
	{
		OpAdd = 1,
		OpSub = 2,
		OpRevSub = 3,
		OpMin = 4,
		OpMax = 5,
		Max
	};

	enum class ERHILogicOperation
	{
		Clear = 0,
		Set = (Clear + 1),
		Copy = (Set + 1),
		CopyInv = (Copy + 1),
		NoOp = (CopyInv + 1),
		Invert = (NoOp + 1),
		And = (Invert + 1),
		Nand = (And + 1),
		Or = (Nand + 1),
		Nor = (Or + 1),
		Xor = (Nor + 1),
		Equiv = (Xor + 1),
		RevAnd = (Equiv + 1),
		InvAnd = (RevAnd + 1),
		RevOr = (InvAnd + 1),
		InvOr = (RevOr + 1),
		Max
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
			def.Cullmode = ERHICullMode::None;
			def.bFrontCounterClockwise = false;
			def.DepthBias = 0;
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
		constexpr static RHIBlendState GetDefault()
		{
			RHIBlendState def{};
			def.bEnableAlphaToCoverage = false;
			def.bEnableIndependentBlend = false;
			return def;
		}

		bool bEnableAlphaToCoverage = false;
		bool bEnableIndependentBlend = false;
	};
}
#pragma endregion

#pragma region RHI Types - Classes
namespace Influx::Graphics
{
	// RHITexture
	using RHITextureHandle = void*;

	struct RHITextureDesc final
	{
		Vectoru2	Dimensions;
		uint16		Mips;
		ERHIFormat	Format;
	};

	// RHIGraphicsCommandList
	using RHIGraphicsCommandListHandle = void*;

	// RHI Graphics Command Queue
	using RHIGraphicsCommandQueueHandle = void*;

	// RHIPipeline
	using RHIGraphicsPipelineHandle = void*;

	struct RHIGraphicsPipelineDesc final
	{
		using CompiledShaderData = Vector<uint8>;

		constexpr static uint8 k_maxNumInputElements = 8u;
		constexpr static uint8 k_maxBoundRenderTargets = 8u;

		struct InputElement final
		{
			InputElement(const String& name, uint8 semanticIndex, ERHIFormat format, uint8 inputSlot, uint8 alignedByteOffset, bool dataPerVertexNotPerInstance, uint8 instanceDataStepRate)
				: SemanticName{ name }, SemanticIndex{ semanticIndex }, Format{ format }, InputSlot{ inputSlot }, AlignedByteOffset{ alignedByteOffset }
				, bDataPerVertexNotPerInstance{ dataPerVertexNotPerInstance }, InstanceDataStepRate{ instanceDataStepRate } {}

			String		SemanticName;
			uint8		SemanticIndex;
			ERHIFormat	Format;
			uint8		InputSlot;
			uint8		AlignedByteOffset;
			bool		bDataPerVertexNotPerInstance;
			uint8		InstanceDataStepRate;
		};

		Array<InputElement, k_maxNumInputElements> InputElements;
		RHIRasterizerState RasterizerState;
		RHIBlendState BlendState;
		RHIDepthStencilState DepthStencilState;
		ERHIPrimitiveTopologyType PrimitiveTopologyType;

		CompiledShaderData VS;
		CompiledShaderData PS;
		CompiledShaderData DS;
		CompiledShaderData HS;
		CompiledShaderData GS;

		uint8 SampleCount = 1u;
		uint8 SampleQuality = 0u;
		uint8 SampleMask = 255u;
		uint8 NodeMask = 0u;

		struct
		{
			ERHIFormat Format = ERHIFormat::INVALID;

			struct
			{
				bool bEnableBlend = false;
				bool bEnableLogicOp = false;

				ERHIBlend SrcBlend = ERHIBlend::One;
				ERHIBlend DestBlend = ERHIBlend::Zero;
				ERHIBlendOperation BlendOperation = ERHIBlendOperation::OpAdd;

				ERHIBlend SrcBlendAlpha = ERHIBlend::One;
				ERHIBlend DestBlendAlpha = ERHIBlend::Zero;
				ERHIBlendOperation BlendOperationAlpha = ERHIBlendOperation::OpAdd;

				ERHILogicOperation LogicOperation = ERHILogicOperation::NoOp;

				uint8 RenderTargetWriteMask = 15u;

			} BlendDesc;

		} RenderTargets[k_maxBoundRenderTargets];
	};

	// RHIPipeline Layout
	using RHIGraphicsPipelineLayoutHandle = void*;

	struct RHIGraphicsPipelineLayoutDesc final
	{

	};

	// RHIBuffer 
	using RHIBufferHandle = void*;

	struct RHIBufferDesc final
	{
		ERHIResourceState State;
		RHIClearValue ClearValue;
		uint64 SizeInBytes;
	};

	// RHISwapchain
	using RHISwapchainHandle = void*;

	struct RHISwapchainDesc final
	{
		bool bIsTearingSupported;
	};

	// RHIDescriptorHeap
	using RHIDescriptorHeapHandle = void*;

	struct RHIDescriptorHeapDesc final
	{
		ERHIResourceViewType Type;
		bool bIsShaderVisible;
		uint64 TotalNumDescriptors;
		uint64 NumDescriptorsOccupied;
	};
}
#pragma endregion

#pragma endregion

namespace Influx::Graphics
{
	struct EResult final
	{
		enum class EMessageLevel
		{
			Info,
			Warning,
			Error,
			Max
		};

		EResult(bool success = true, EMessageLevel messageLevel = EMessageLevel::Info) 
			: bSuccess{ success } {}

		operator bool() const
		{
			return bSuccess == true;
		}

		bool bSuccess = true;
		EMessageLevel MessageLevel = EMessageLevel::Info;
	};

	const static EResult FailWarning	= EResult(false, EResult::EMessageLevel::Warning);
	const static EResult FailError		= EResult(false, EResult::EMessageLevel::Error);
}

namespace Influx::Graphics
{
	/* Initialize resources for a given EGraphicsAPI */
	static EResult Initialize(EGraphicsAPI api);

	/* Clean up resources that are tied to the currently initialized EGraphicsAPI */
	static EResult Cleanup();

	/* Returns the currently initialized Graphics API */
	static EGraphicsAPI GetInitializedGraphicsAPI();

	/* */
	static EResult SetDebugLayerEnabled();

	/* */
	static bool IsDebugLayerEnabled();

	/* Create a scope in which 'internalFunc' gets executed */
	/* Within the scope the given 'api' is initialized, outside of the scope, we clean it up */
	static EResult Create(EGraphicsAPI api, Function<void()> internalFunc)
	{
		EResult result = EResult();

		if (internalFunc != nullptr)
		{
			if (!(result = Initialize(api)))
			{
				return result;
			}

			internalFunc();

			if (!(result = Cleanup()))
			{
				return result;
			}
		}

		return result;
	}
	
	/* */
	static EResult CreateGraphicsCommandQueue(RHIGraphicsCommandQueueHandle& out_handle);

	/* */
	static EResult CreateGraphicsCommandList(RHIGraphicsCommandListHandle& out_handle);

	/* */
	static EResult CreateSwapchain(RHISwapchainHandle& out_handle);

	/* */
	static EResult CreateDescriptorHeap(const RHIDescriptorHeapDesc& desc, RHIDescriptorHeapHandle& out_handle);

	/* */
	static EResult CreateGraphicsPipeline(const RHIGraphicsPipelineDesc& desc, RHIGraphicsPipelineHandle& out_handle);

	/* */
	static EResult CreateGraphicsPipelineLayout(const RHIGraphicsPipelineLayoutDesc& desc, RHIGraphicsPipelineLayoutHandle& out_handle);

	/* */
	static EResult CreateBuffer(const RHIBufferDesc& desc, RHIBufferHandle& out_handle);
}

#endif