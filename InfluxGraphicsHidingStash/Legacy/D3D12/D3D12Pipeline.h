#pragma once

#include "InfluxGraphics/RHIPipeline.h"
#include "D3D12.h"

namespace Influx::Graphics
{
	class D3D12GraphicsPipelineLayout final : public RHIGraphicsPipelineLayout
	{
		friend class D3D12API;

	public:
		ID3D12RootSignature* GetDxRootSignature() const;
		virtual ~D3D12GraphicsPipelineLayout();

	private:
		ID3D12RootSignature* DxRootSignature;

		D3D12GraphicsPipelineLayout() = default;
	};

	class D3D12GraphicsPipeline final : public RHIGraphicsPipeline
	{
		friend class D3D12API;

	public:
		virtual ~D3D12GraphicsPipeline();

		ID3D12PipelineState* GetDxPipelineState() const;

	private:
		ID3D12PipelineState* DxPipelineState;

#pragma region StateStreamStructure
#pragma warning(push)
#pragma warning(disable : 4324)
		struct StateStream
		{
#pragma region TypeDefs
			template<typename TInnerStructType, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE TType, typename DefaultArg = TInnerStructType>
			struct Subobject
			{
				Subobject() noexcept : Inner(DefaultArg()) {}
				Subobject(const TInnerStructType& o) noexcept : Inner(o) {}
				Subobject& operator=(const TInnerStructType& o) noexcept { Inner = o; return *this; }
				operator TInnerStructType const& () const noexcept { return Inner; }
				operator TInnerStructType& () noexcept { return Inner; }
				TInnerStructType* operator&() noexcept { return &Inner; }
				TInnerStructType const* operator&() const noexcept { return &Inner; }

				D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type = TType;
				TInnerStructType Inner;
			};
			using FLAGS = Subobject<D3D12_PIPELINE_STATE_FLAGS, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_FLAGS>;
			using NODE_MASK = Subobject<UINT, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_NODE_MASK>;
			using ROOT_SIGNATURE = Subobject<ID3D12RootSignature*, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE>;
			using INPUT_LAYOUT = Subobject<D3D12_INPUT_LAYOUT_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT>;
			using IB_STRIP_CUT_VALUE = Subobject<D3D12_INDEX_BUFFER_STRIP_CUT_VALUE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_IB_STRIP_CUT_VALUE>;
			using PRIMITIVE_TOPOLOGY = Subobject<D3D12_PRIMITIVE_TOPOLOGY_TYPE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY>;
			using VS = Subobject<D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS>;
			using GS = Subobject<D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_GS>;
			using STREAM_OUTPUT = Subobject<D3D12_STREAM_OUTPUT_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_STREAM_OUTPUT>;
			using HS = Subobject<D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_HS>;
			using DS = Subobject<D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DS>;
			using PS = Subobject<D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS>;
			using AS = Subobject<D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS>;
			using MS = Subobject<D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS>;
			using CS = Subobject<D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CS>;
			using BLEND_DESC = Subobject<D3D12_BLEND_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND>;
			using DEPTH_STENCIL = Subobject<D3D12_DEPTH_STENCIL_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL>;
			using DEPTH_STENCIL1 = Subobject<D3D12_DEPTH_STENCIL_DESC1, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL1>;
			using DEPTH_STENCIL_FORMAT = Subobject<DXGI_FORMAT, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT>;
			using RASTERIZER = Subobject<D3D12_RASTERIZER_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER>;
			using RENDER_TARGET_FORMATS = Subobject<D3D12_RT_FORMAT_ARRAY, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS>;
			using SAMPLE_DESC = Subobject<DXGI_SAMPLE_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC>;
			using SAMPLE_MASK = Subobject<UINT, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_MASK>;
			using CACHED_PSO = Subobject<D3D12_CACHED_PIPELINE_STATE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CACHED_PSO>;
			using VIEW_INSTANCING = Subobject < D3D12_VIEW_INSTANCING_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VIEW_INSTANCING>;
#pragma endregion
			// Supported Members...
			ROOT_SIGNATURE RootSignature;
			INPUT_LAYOUT InputLayout;
			PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
			VS VertexShader;
			PS PixelShader;
			DEPTH_STENCIL_FORMAT DsvFormat;
			RENDER_TARGET_FORMATS RtvFormats;
			RASTERIZER Rasterizer;
			DEPTH_STENCIL DepthStencil;

		} PipelineStateStream{};
#pragma warning(pop)
#pragma endregion

		D3D12GraphicsPipeline() = default;
	};
}