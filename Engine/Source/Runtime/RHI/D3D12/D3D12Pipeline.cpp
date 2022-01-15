#include "pch.h"
#include "D3D12Pipeline.h"

#include "D3D12RootSignature.h"

#include "Core/Type/Type.h"

namespace Influx
{
	Ptr<D3D12GraphicsPipeline> D3D12GraphicsPipeline::Create(const Ptr<D3D12API> api, Ptr<D3D12RootSignature> rootSignatureRef, const GraphicsPipelineBuilder& builder)
	{
		Ptr<D3D12GraphicsPipeline> newPipeline = new D3D12GraphicsPipeline(builder);
		newPipeline->mpRootSignatureRef = rootSignatureRef;
		newPipeline->mPipelineStateStream.RootSignature = rootSignatureRef->GetD3D12RootSignature();

		auto device = api->GetDevice();

		/* Configure the PipelineState... */

		/* Define InputLayout*/
		D3D12_INPUT_ELEMENT_DESC inputLayout[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		};
		newPipeline->mPipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };

		
		switch (builder.PrimitiveTopologyType)
		{
		case ERHIPrimitiveTopologyType::Triangle: newPipeline->mPipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; break;
		default: newPipeline->mPipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; break;
		}

		D3D12_RT_FORMAT_ARRAY rtvFormats{};
		rtvFormats.NumRenderTargets = (uint32_t)builder.RenderTargetViewFormats.size();
		for (int i{}; i < builder.RenderTargetViewFormats.size(); ++i)
		{
			ERHIFormat format = builder.RenderTargetViewFormats[i];
			switch (format)
			{
			case ERHIFormat::RGBA_8_Unorm: rtvFormats.RTFormats[i] = DXGI_FORMAT_R8G8B8A8_UNORM; break;
			default: rtvFormats.RTFormats[i] = DXGI_FORMAT_R8G8B8A8_UNORM; break;
			}
		}
		newPipeline->mPipelineStateStream.RTVFormats = rtvFormats;

		switch (builder.DepthStencilViewFormat)
		{
		case ERHIFormat::D_32_Float: newPipeline->mPipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT; break;
		case ERHIFormat::INVALID: newPipeline->mPipelineStateStream.DSVFormat = DXGI_FORMAT_UNKNOWN; break;
		default: newPipeline->mPipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT; break;
		}

		// [ULTRA-CRINGE]
		{
			// Load the vertex shader.
			ID3DBlob* vertexShaderBlob;
			ID3DBlob* pixelShaderBlob;
			D3DReadFileToBlob(ToWString(builder.VSShaderPath).c_str(), &vertexShaderBlob);
			D3DReadFileToBlob(ToWString(builder.PSShaderPath).c_str(), &pixelShaderBlob);

			newPipeline->mPipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob);
			newPipeline->mPipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob);
		}

		/* Initialize PSS object */
		const D3D12_PIPELINE_STATE_STREAM_DESC pssDesc{ sizeof(PipelineStateStream), &newPipeline->mPipelineStateStream};
		HRESULT res;
		res = device->CreatePipelineState(&pssDesc, IID_PPV_ARGS(&newPipeline->mpD3D12PipelineStateObject));
		if (FAILED(res))
		{
			/* TODO */
		}

		return newPipeline;
	}

	D3D12GraphicsPipeline::D3D12GraphicsPipeline(const GraphicsPipelineBuilder& desc) : RHIGraphicsPipeline(desc), mpD3D12PipelineStateObject{ nullptr }{}

	Ptr<ID3D12PipelineState> D3D12GraphicsPipeline::GetD3D12PipelineState() const
	{
		return mpD3D12PipelineStateObject;
	}

	Ptr<D3D12RootSignature> D3D12GraphicsPipeline::GetRootSignatureRef() const
	{
		return mpRootSignatureRef;
	}
}
