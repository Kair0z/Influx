#pragma once
#include "Runtime/RHI/Pipeline.h"
#include "D3D12API.h"

namespace Influx
{
	class D3D12RootSignature;

	/* Pipeline State Stream... */
	struct PipelineStateStream
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE RootSignature{};
		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout{};
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType{};
		CD3DX12_PIPELINE_STATE_STREAM_VS VS{};
		CD3DX12_PIPELINE_STATE_STREAM_PS PS{};
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat{};
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats{};
	};

	/* Wrapper around ID3D12PipelineState */
	class D3D12GraphicsPipeline : public RHIGraphicsPipeline
	{
	public:
		static Ptr<D3D12GraphicsPipeline> Create(const Ptr<D3D12API> api, Ptr<D3D12RootSignature> rootSignatureRef, const GraphicsPipelineBuilder& builder);

		Ptr<ID3D12PipelineState> GetD3D12PipelineState() const;
		Ptr<D3D12RootSignature> GetRootSignatureRef() const;

	private:
		D3D12GraphicsPipeline(const GraphicsPipelineBuilder& desc);

		ID3D12PipelineState* mpD3D12PipelineStateObject;
		Ptr<D3D12RootSignature> mpRootSignatureRef;

		PipelineStateStream mPipelineStateStream{};
	};
}


