#include "D3D12Pipeline.h"
#include "D3D12Conversion.h"
#include "D3D12Shader.h"

namespace Influx::Graphics
{
	/* API Creation Functions */
	RHIGraphicsPipelineLayout* D3D12API::CreateGraphicsPipelineLayout(const RHIGraphicsPipelineLayoutDescription& constructionArgs) const
	{
		D3D12GraphicsPipelineLayout* graphicsPipelineLayout = new D3D12GraphicsPipelineLayout();
		graphicsPipelineLayout->ConstructionDescription = constructionArgs;

		// Check for Root Signature Feature support...
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData{};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		if (FAILED(DxDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		// Allow input layout and deny unnecessary access to certain pipeline stages...
		D3D12_ROOT_SIGNATURE_FLAGS flags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		std::vector<D3D12_ROOT_PARAMETER1> rootSigParams{};
		std::vector<D3D12_STATIC_SAMPLER_DESC> rootSigStaticSamplers{};
		const std::vector<std::shared_ptr<Internal::BaseResourceBinding>>& rhiResourceBindings = constructionArgs.LayoutBindings.ResourceBindings;
		for (const std::shared_ptr<Internal::BaseResourceBinding>& rhiBinding : rhiResourceBindings)
		{
			D3D12_ROOT_PARAMETER1 newDxParam{};
			newDxParam.ParameterType = Conversion::ToDx12(rhiBinding->GetBindingType());
			newDxParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // Todo: Not good!

			switch (rhiBinding->GetBindingType())
			{
			case ERHIResourceBindingType::Constants:
				newDxParam.Constants.Num32BitValues = rhiBinding->GetNum();
				newDxParam.Constants.RegisterSpace = rhiBinding->GetBindingSpace();
				newDxParam.Constants.ShaderRegister = 0; // Todo: ???

				break;

			case ERHIResourceBindingType::CBV:
			case ERHIResourceBindingType::SRV:
			case ERHIResourceBindingType::UAV:
				newDxParam.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
				newDxParam.Descriptor.RegisterSpace = rhiBinding->GetBindingSpace();
				newDxParam.Descriptor.ShaderRegister = 0; // Todo: ???
				break;

			default: // Todo: Do we care? Is this possible? // !! Descriptor Tables !!
				break;
			}
			rootSigParams.push_back(newDxParam);
		}

		constexpr static D3D12_ROOT_SIGNATURE_FLAGS DefaultFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;

		D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Version = featureData.HighestVersion;
		rootSignatureDesc.Desc_1_1.NumParameters = (uint32_t)rootSigParams.size();
		rootSignatureDesc.Desc_1_1.pParameters = rootSigParams.data();
		rootSignatureDesc.Desc_1_1.NumStaticSamplers = (uint32_t)rootSigStaticSamplers.size();
		rootSignatureDesc.Desc_1_1.pStaticSamplers = rootSigStaticSamplers.data();
		rootSignatureDesc.Desc_1_1.Flags = DefaultFlags;

		ID3DBlob* rootSignatureBlob;
		ID3DBlob* errorBlob;
		D3D12API::SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &rootSignatureBlob, &errorBlob);
		HRESULT res = DxDevice->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&graphicsPipelineLayout->DxRootSignature));
		if (!SUCCEEDED(res))
		{
			assert(false);
		}

		return graphicsPipelineLayout;
	}

	RHIGraphicsPipeline* D3D12API::CreateGraphicsPipeline(const RHIGraphicsPipelineDescription& constructionArgs, RHIGraphicsPipelineLayout* pipelineLayoutReference) const
	{
		D3D12GraphicsPipeline* d3d12Pipeline = new D3D12GraphicsPipeline();
		D3D12GraphicsPipelineLayout* d3d12Layout = (D3D12GraphicsPipelineLayout*)pipelineLayoutReference;
		d3d12Pipeline->PipelinelayoutReference = d3d12Layout;
		d3d12Pipeline->ConstructionDescription = constructionArgs;

		D3D12GraphicsPipeline::StateStream& stateStream = d3d12Pipeline->PipelineStateStream;

		// Root Signature
		stateStream.RootSignature = d3d12Layout->DxRootSignature;

		// InputLayout
		D3D12_INPUT_ELEMENT_DESC inputLayout[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		};
		stateStream.InputLayout = { inputLayout, _countof(inputLayout) };

		// Primitive Topology Type
		stateStream.PrimitiveTopologyType = Conversion::ToDx12(constructionArgs.PrimitiveTopologyType);

		// RTV Formats
		D3D12_RT_FORMAT_ARRAY rtvFormats{};
		rtvFormats.NumRenderTargets = static_cast<UINT>(constructionArgs.RTVFormats.size());
		for (size_t i = 0; i < rtvFormats.NumRenderTargets; ++i)
		{
			rtvFormats.RTFormats[i] = Conversion::ToDx12(constructionArgs.RTVFormats[i]);
		}
		stateStream.RtvFormats = rtvFormats;

		// DSV Single Format
		stateStream.DsvFormat = Conversion::ToDx12(constructionArgs.DSVFormat);

		// Depth Stencil
		D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
		depthStencilDesc.DepthEnable = constructionArgs.bDepthEnabled;
		depthStencilDesc.StencilEnable = constructionArgs.bStencilEnabled;
		stateStream.DepthStencil = depthStencilDesc;

		// Shaders:
		for (RHIShader* shader : { constructionArgs.PixelShader, constructionArgs.VertexShader })
		{
			D3D12_SHADER_BYTECODE shaderByteCode{};
			shaderByteCode.BytecodeLength = shader->GetShaderDataNumBytes();
			shaderByteCode.pShaderBytecode = shader->GetShaderDataPtr();

			switch (shader->GetType())
			{
			default:
			case ERHIShaderType::PixelShader:
				stateStream.PixelShader = shaderByteCode;
				break;

			case ERHIShaderType::VertexShader:
				stateStream.VertexShader = shaderByteCode;
				break;
			}
		}

		// Rasterizer
		D3D12_RASTERIZER_DESC rasterDesc{};
		rasterDesc.CullMode = Conversion::ToDx12(constructionArgs.RasterCullMode);
		rasterDesc.FillMode = Conversion::ToDx12(constructionArgs.RasterFillMode);
		rasterDesc.ConservativeRaster = (constructionArgs.bConservativeRaster) ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		rasterDesc.DepthBias = constructionArgs.RasterDepthBias;
		rasterDesc.DepthBiasClamp = constructionArgs.RasterMaxDepthBias;
		rasterDesc.DepthClipEnable = constructionArgs.bRasterDepthClipEnable;
		rasterDesc.AntialiasedLineEnable = constructionArgs.bAntialiasedLineEnable;
		stateStream.Rasterizer = rasterDesc;

		const D3D12_PIPELINE_STATE_STREAM_DESC streamDesc{ sizeof(D3D12GraphicsPipeline::StateStream), &d3d12Pipeline->PipelineStateStream };
		HRESULT res = DxDevice->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&d3d12Pipeline->DxPipelineState));
		if (FAILED(res))
		{
			assert(false);
		}

		return d3d12Pipeline;
	}

	RHIGraphicsPipeline* D3D12API::CreateGraphicsPipeline(const RHIGraphicsPipelineDescription& constructionArgs, RHIGraphicsPipelineLayout* pipelineLayoutReference, RHIRenderPass* renderPass) const
	{
		return CreateGraphicsPipeline(constructionArgs, pipelineLayoutReference); // D3D12 and my sanity is clearly not ready for RenderPass implementation :D
	}

	ID3D12RootSignature* D3D12GraphicsPipelineLayout::GetDxRootSignature() const
	{
		return DxRootSignature;
	}

	D3D12GraphicsPipelineLayout::~D3D12GraphicsPipelineLayout()
	{
		D3D12API::SafeRelease(DxRootSignature);
	}

	D3D12GraphicsPipeline::~D3D12GraphicsPipeline()
	{
		D3D12API::SafeRelease(DxPipelineState);
	}

	ID3D12PipelineState* D3D12GraphicsPipeline::GetDxPipelineState() const
	{
		return DxPipelineState;
	}
}