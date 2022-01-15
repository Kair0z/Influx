#include "pch.h"
#include "D3D12RootSignature.h"

namespace Influx
{
	Ptr<D3D12RootSignature> D3D12RootSignature::Create(const Ptr<D3D12API> api, const D3D12RootSignatureDesc& desc)
	{
		D3D12RootSignature* newSignature = new D3D12RootSignature();
		auto device = api->GetDevice();

		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData{};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
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

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(
			(uint32_t)desc.Parameters.size(), desc.Parameters.data(), 
			(uint32_t)desc.StaticSamplers.size(), desc.StaticSamplers.data(), flags);

		ID3DBlob* rootSignatureBlob;
		ID3DBlob* errorBlob;
		D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &rootSignatureBlob, &errorBlob);

		/* Create Root Signature */
		device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&newSignature->mpD3D12RootSignature));

		return newSignature;
	}

	Ptr<ID3D12RootSignature> D3D12RootSignature::GetD3D12RootSignature() const
	{
		return mpD3D12RootSignature;
	}
}

