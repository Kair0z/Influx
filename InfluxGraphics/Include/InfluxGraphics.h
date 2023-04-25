#pragma once

#ifndef __INFLUX_GRAPHICS_H_
#define __INFLUX_GRAPHICS_H_

#pragma comment (lib, "InfluxGraphics.lib")

// Defines
#pragma region Defines

#ifndef INFLUX_GRAPHICS_API
#define INFLUX_GRAPHICS_API
#endif

#define INFLUX_GRAPHICS_USE_CORE		1
#define INFLUX_GRAPHICS_USE_STL			1

#define INFLUX_GRAPHICS_INCLUDE_DX12	1
#define INFLUX_GRAPHICS_INCLUDE_VULKAN	1

#define INFLUX_GRAPHICS_DEBUG			_DEBUG

#ifndef INFLUX_GRAPHICS_TODO
#if INFLUX_GRAPHICS_DEBUG
#include <cassert>

#define INFLUX_GRAPHICS_TODO __debugbreak();
#define INFLUX_GRAPHICS_ASSERT(x) assert(x);

#else
#define INFLUX_GRAPHICS_TODO
#define INFLUX_GRAPHICS_ASSERT
#endif
#endif // INFLUX_GRAPHICS_TODO

#pragma endregion

#pragma region Predeclarations

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
#endif // INFLUX_GRAPHICS_USE_CORE

#if INFLUX_GRAPHICS_USE_STL
#include <vector>
#include <array>
#include <string>
#endif // INFLUX_GRAPHICS_USE_STL

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
#endif // INFLUX_GRAPHICS_USE_CORE
}
#pragma endregion

#pragma region RHI Types - Enum
namespace Influx::Graphics
{
	/* Graphics APIs */
	enum class EGraphicsAPI : uint8
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
		PixelShader,
		Max
	};

	enum class ERHIShaderStageFlags
	{
		Default,
		Max
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
		_1	= 1u,
		_2	= 2u,
		_4	= 4u,
		_8	= 8u,
		_16	= 16u,
		_32	= 32u,
		_64	= 64u,
		Max
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
		Triangle,
		Max
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
		FrontFaceCull,
		Max
	};

	enum class ERHIFillMode
	{
		Solid,
		Wireframe,
		Max
	};

	enum class ERHIPipelineBindPoint
	{
		Graphics,
		Max
	};

	enum class ERHIShaderModel
	{
		SM_5_0,
		Max
	};

	enum class ERHIComparisonFunc
	{
		Never		= 1u,
		Less		= 2u,
		Equal		= 3u,
		LessEq		= 4u,
		Greater		= 5u,
		NotEqual	= 6u,
		GreaterEq	= 7u,
		Always		= 8u,
		Max
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
		Color,
		Max
	};

	enum class ERHIRenderPassLoadOp
	{
		Load,			// Preserve the existing contents of the attachment
		Clear,			// Clear the values to a constant at the start
		DontCare,		// Existing contents are undefined; we don't care about them
		Max
	};

	enum class ERHIRenderPassStoreOp
	{
		Store,			// Rendered contents will be stored in memory and can be read later
		DontCare,		// Contents of the framebuffer will be undefined after the rendering operation
		Max
	};

	struct RHIClearValue final
	{
		static RHIClearValue Default()
		{
			RHIClearValue result{};
			return result;
		}

		Math::Vectorf4 Colour;
		float Depth;
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
	/* Every type of object created by the RHI */
	enum class ERHIChild : uint8
	{
		CommandQueue,
		CommandList,
		CommandBuffer,
		Swapchain,
		GraphicsPipeline,
		GraphicsPipelineLayout,
		Resource,
		DescriptorHeap,
		Descriptor,
		Fence,
		Max
	};

	constexpr uint8 k_numRHIObjectTypes = static_cast<uint8>(ERHIChild::Max);
	
	constexpr static uint8 k_maxNumSamplerDescriptorsPerHeap = 16u;
	constexpr static uint8 k_maxNumResourceDescriptorsPerHeap = 64u;
	constexpr static uint8 k_maxNumRtvDescriptorsPerHeap = 64u;
	constexpr static uint8 k_maxNumDsvDescriptorsPerHeap = 64u;
	constexpr static uint8 k_maxNumDescriptors = k_maxNumSamplerDescriptorsPerHeap + k_maxNumResourceDescriptorsPerHeap + k_maxNumRtvDescriptorsPerHeap + k_maxNumDsvDescriptorsPerHeap;

	constexpr uint8 k_maxNumRHIObjectsPerType[k_numRHIObjectTypes]
	{
		3u,						// CommandQueue
		64u,					// CommandList
		16u,					// CommandBuffer
		1u,						// Swapchain
		64u,					// GraphicsPipeline
		64u,					// GraphicsPipelineLayout
		255u,					// Resource
		4u,						// DescriptorHeap
		k_maxNumDescriptors,	// Descriptor
		64u						// Fence
	};

	constexpr uint8 GetMaxNumOfObjects(const ERHIChild child)
	{
		return k_maxNumRHIObjectsPerType[static_cast<uint8>(child)];
	}

	constexpr const char* k_RHIObjectsNameStrings[k_numRHIObjectTypes]
	{
		"CommandQueue",
		"CommandList",
		"CommandBuffer",
		"Swapchain",
		"GraphicsPipeline",
		"GraphicsPipelineLayout",
		"Resource",
		"DescriptorHeap",
		"Descriptor",
		"Fence"
	};

	constexpr const char* GetRHIObjectTypeString(const ERHIChild child)
	{
		return k_RHIObjectsNameStrings[static_cast<uint8>(child)];
	}

	constexpr uint8 GetMaxNumDescriptorsPerHeap(const ERHIDescriptorType descriptorHeapType)
	{
		switch (descriptorHeapType)
		{
		case ERHIDescriptorType::Resource:	return k_maxNumResourceDescriptorsPerHeap;
		case ERHIDescriptorType::Sampler:	return k_maxNumSamplerDescriptorsPerHeap;
		case ERHIDescriptorType::RTV:		return k_maxNumRtvDescriptorsPerHeap;
		case ERHIDescriptorType::DSV:		return k_maxNumDsvDescriptorsPerHeap;
		}

		return 0u;
	}

	// Handles: 
	// Wrappers that keep a raw void* pointing pointing to the created API-specific object
	class IRHIObjectHandle
	{
	public:
		virtual ERHIChild GetType() const { return ERHIChild::Max; };

		operator bool() const
		{
			return IsValid();
		}

		bool IsValid() const
		{
			return mp_internal != nullptr;
		}

		void* GetInternal() const
		{
			return mp_internal;
		}

		template <class _T>
		_T* GetInternal() const
		{
			return static_cast<_T*>(mp_internal);
		}

		template <class _T>
		_T* As() const
		{
			return static_cast<_T*>(mp_internal);
		}

#if INFLUX_GRAPHICS_DEBUG
		virtual const char* GetDebugName() const { return "IRHIObject"; };
#endif

	protected:
		IRHIObjectHandle() = default;
		IRHIObjectHandle(void* pInternal) : mp_internal{ pInternal } {}
	private:
		uint32 a = 2u;
		void* mp_internal = nullptr;
	};
	template <ERHIChild _E>
	struct RHIObjectHandle : public IRHIObjectHandle 
	{
		// We can publicly create invalid handles.
		RHIObjectHandle() = default;
		
#if INFLUX_GRAPHICS_DEBUG
		virtual const char* GetDebugName() const override final { return k_RHIObjectsNameStrings[static_cast<uint8>(_E)]; };
#endif

		virtual ERHIChild GetType() const override final { return GetStaticType(); };

		constexpr static ERHIChild GetStaticType() { return _E; }

		constexpr static RHIObjectHandle<_E> GetInvalid()
		{
			return RHIObjectHandle(nullptr);
		}

	protected:
		RHIObjectHandle(void* internalPointer) 
			: IRHIObjectHandle(internalPointer) {}

		friend class GlobalState;
	};

	using RHICommandListHandle				= RHIObjectHandle<ERHIChild::CommandList>;
	using RHICommandQueueHandle				= RHIObjectHandle<ERHIChild::CommandQueue>;
	using RHICommandBufferHandle			= RHIObjectHandle<ERHIChild::CommandBuffer>;
	using RHIGraphicsPipelineHandle			= RHIObjectHandle<ERHIChild::GraphicsPipeline>;
	using RHIGraphicsPipelineLayoutHandle	= RHIObjectHandle<ERHIChild::GraphicsPipelineLayout>;
	using RHIResourceHandle					= RHIObjectHandle<ERHIChild::Resource>;
	using RHIDescriptorHeapHandle			= RHIObjectHandle<ERHIChild::DescriptorHeap>;
	using RHIDescriptorHandle				= RHIObjectHandle<ERHIChild::Descriptor>;
	using RHISwapchainHandle				= RHIObjectHandle<ERHIChild::Swapchain>;
	using RHIFenceHandle					= RHIObjectHandle<ERHIChild::Fence>;

	// Descriptors: 
	// describe construction & creation of RHI-objects
	struct IRHIDesc
	{
	public:
		IRHIDesc() = default;

#if INFLUX_GRAPHICS_DEBUG
		virtual const char* GetDebugName() const { return "IRHIDesc"; };
#endif

		virtual ERHIChild GetType() const { return ERHIChild::Max; }
	};
	template <ERHIChild _E>
	struct RHIDesc : public IRHIDesc
	{
		RHIDesc() = default;

#if INFLUX_GRAPHICS_DEBUG
		constexpr static const char* GetStaticDebugName() { return GetRHIObjectTypeString(_E); }
		virtual const char* GetDebugName() const override final { return GetStaticDebugName(); };
#endif

		constexpr static ERHIChild GetStaticType() { return _E; }
		virtual ERHIChild GetType() const override final { return GetStaticType(); };
	};

	struct RHIGraphicsPipelineDesc final : public RHIDesc<ERHIChild::GraphicsPipeline>
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

	struct RHIGraphicsPipelineLayoutDesc final : public RHIDesc<ERHIChild::GraphicsPipelineLayout>
	{

	};

	struct RHIBufferDesc final : public RHIDesc<ERHIChild::Resource>
	{
		ERHIResourceState State;
		RHIClearValue ClearValue;
		uint64 SizeInBytes;
	};

	struct RHISwapchainDesc final : public RHIDesc<ERHIChild::Swapchain>
	{
		enum class EBuffering : uint8
		{
			Single = 1,
			Double = 2,
			Triple = 3,
			Max
		};

		uint8 GetNumBuffers() const
		{
			switch (Buffering)
			{
			case EBuffering::Single: return 1u;
			case EBuffering::Double: return 2u;
			case EBuffering::Triple: return 3u;

			default:
				return 0u;
			}
		}
		
		EBuffering Buffering;

		void* WindowHandle;

		Math::Vectoru2 Dimensions;

		bool bIsTearingSupported;

		ERHIFormat RenderTargetFormat;
	};

	struct RHIDescriptorHeapDesc final : public RHIDesc<ERHIChild::DescriptorHeap>
	{
		ERHIResourceViewType Type;
		bool bIsShaderVisible;
		uint64 TotalNumDescriptors;
		uint64 NumDescriptorsOccupied;
	};
}
#pragma endregion

#pragma endregion

// Result
namespace Influx::Graphics
{
	struct Result final
	{
		enum class EMessageLevel
		{
			Info,
			Warning,
			Error,
			Max
		};

		static Result Success()
		{
			return Result(true);
		}

		static Result Fail()
		{
			return Result(false);
		}

		Result(bool success = true, EMessageLevel messageLevel = EMessageLevel::Info) 
			: bSuccess{ success } {}

		operator bool() const
		{
			return bSuccess == true;
		}

		bool bSuccess = true;
		EMessageLevel MessageLevel = EMessageLevel::Info;
	};
}

// [MAIN API]
namespace Influx::Graphics
{
	INFLUX_GRAPHICS_API Result RegisterNative(EGraphicsAPI api, ERHIChild type, void* ptr);

	/* Initialize resources for a given EGraphicsAPI */
	INFLUX_GRAPHICS_API Result Initialize(EGraphicsAPI api);

	/* Clean up resources that are tied to the currently initialized EGraphicsAPI */
	INFLUX_GRAPHICS_API Result Cleanup();

	/* Create a scope in which 'internalFunc' gets executed */
	/* Within the scope the given 'api' is initialized, outside of the scope, we clean it up */
	INFLUX_GRAPHICS_API Result Create(EGraphicsAPI api, Function<void()> internalFunc);

	/* Returns the currently initialized Graphics API */
	INFLUX_GRAPHICS_API EGraphicsAPI GetInitializedGraphicsAPI();

	/* */
	INFLUX_GRAPHICS_API Result SetDebugLayerEnabled();

	/* */
	INFLUX_GRAPHICS_API bool IsDebugLayerEnabled();
	

	/* Creates a Compute command queue OR gets an existing one */
	INFLUX_GRAPHICS_API Result GetComputeCommandQueue(RHICommandQueueHandle& out_handle);

	/* Creates a Graphics command queue OR gets an existing one */
	INFLUX_GRAPHICS_API Result GetGraphicsCommandQueue(RHICommandQueueHandle& out_handle);

	/* */
	INFLUX_GRAPHICS_API Result CreateComputeCommandQueue(RHICommandQueueHandle& out_handle);

	/* */
	INFLUX_GRAPHICS_API Result CreateGraphicsCommandQueue(RHICommandQueueHandle& out_handle);

	/* */
	INFLUX_GRAPHICS_API Result GetGraphicsCommandBuffer(RHICommandBufferHandle& out_handle, uint64 fenceValue);

	/* */
	INFLUX_GRAPHICS_API Result GetComputeCommandBuffer(RHICommandBufferHandle& out_handle, uint64 fenceValue);

	/* */
	INFLUX_GRAPHICS_API Result CreateComputeCommandBuffer(RHICommandBufferHandle& out_handle);

	/* */
	INFLUX_GRAPHICS_API Result CreateGraphicsCommandBuffer(RHICommandBufferHandle& out_handle);

	/* */
	INFLUX_GRAPHICS_API Result CreateComputeCommandList(RHICommandBufferHandle& out_existingCommandBuffer, RHICommandListHandle& out_handle);

	/* */
	INFLUX_GRAPHICS_API Result CreateGraphicsCommandList(RHICommandBufferHandle& out_existingCommandBuffer, RHICommandListHandle& out_handle);

	/* */
	INFLUX_GRAPHICS_API Result ResetGraphicsCommandlist(const RHICommandListHandle& commandListHandle, const RHICommandBufferHandle& commandbufferHandle);
	
	/* Waits for the global graphics command queue to reach signal-value */
	INFLUX_GRAPHICS_API Result WaitForGraphicsQueueSignal(const RHICommandQueueHandle& commandQueue, uint64 valueToWaitFor);

	/* Waits for the global graphics command queue to finish ALL work */
	INFLUX_GRAPHICS_API Result WaitForAllGraphicsCommandsFinished();

	/* */
	INFLUX_GRAPHICS_API Result DispatchGraphicsCommands(Function<void(const RHICommandListHandle&)> commands);

	/* */
	INFLUX_GRAPHICS_API Result DispatchComputeCommands(Function<void(const RHICommandListHandle&)> commands);

	/* */
	INFLUX_GRAPHICS_API Result DispatchComputeCommandListToGpu(const RHICommandListHandle& commandListHandle, const RHICommandQueueHandle& commandQueueHandle);

	/* */
	INFLUX_GRAPHICS_API Result DispatchGraphicsCommandListToGpu(const RHICommandListHandle& commandListHandle, const RHICommandQueueHandle& commandQueueHandle);


	/* Create an RHI swapchain 
	* Implicitly creates/uses a RHICommandQueue
	* Implicitly creates RHIBuffer(s) x num-buffers
	* Implicitly creates RHIRenderTargetView(s) x num-buffers
	*/
	INFLUX_GRAPHICS_API Result CreateSwapchain(const RHISwapchainDesc& desc, RHISwapchainHandle& out_handle);

	/* */
	struct PresentDescription final
	{
		bool Vsync = false;
	};

	INFLUX_GRAPHICS_API Result DispatchSwapchainPresent(const RHISwapchainHandle& swapchain, const PresentDescription& present);


	/* */
	INFLUX_GRAPHICS_API Result CreateDescriptorHeap(const RHIDescriptorHeapDesc& desc, RHIDescriptorHeapHandle& out_handle);

	/* Create a Descriptor heap OR gets on that has been created of ERHIDescriptorType */
	INFLUX_GRAPHICS_API Result GetDescriptorHeap(const ERHIDescriptorType type, RHIDescriptorHeapHandle& out_handle);

	/* */
	INFLUX_GRAPHICS_API Result CreateRenderTargetView(const RHIDescriptorHeapHandle& descriptorHeap, const RHIResourceHandle& bufferHandle, RHIDescriptorHandle& out_handle);

	/* Uses GetDescriptorHeap() for convenience */
	INFLUX_GRAPHICS_API Result CreateRenderTargetView(const RHIResourceHandle& bufferHandle, RHIDescriptorHandle& out_handle);


	/* */
	INFLUX_GRAPHICS_API Result CreateGraphicsPipeline(const RHIGraphicsPipelineDesc& desc, RHIGraphicsPipelineHandle& out_handle);

	/* */
	INFLUX_GRAPHICS_API Result CreateGraphicsPipelineLayout(const RHIGraphicsPipelineLayoutDesc& desc, RHIGraphicsPipelineLayoutHandle& out_handle);

	/* */
	INFLUX_GRAPHICS_API Result CreateBuffer(const RHIBufferDesc& desc, RHIResourceHandle& out_handle);


	namespace Commands
	{
		/* */
		INFLUX_GRAPHICS_API Result ClearRenderTargetView(const RHICommandListHandle& cmdListHandle);

		/* */
		INFLUX_GRAPHICS_API Result ClearSwapchainBackBuffer(const RHICommandListHandle& cmdListHandle, const RHISwapchainHandle& swapchainHandle, const Math::Vectorf4& colour);

		namespace Compute
		{
			
		}
	}
}

#endif