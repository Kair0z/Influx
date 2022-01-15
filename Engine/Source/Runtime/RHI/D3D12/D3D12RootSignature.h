#pragma once

#include "D3D12API.h"

#include "Core/Container/Vector.h"
#include "Core/Memory/Reference.h"

namespace Influx
{
	struct D3D12DescriptorRange final
	{
		constexpr static inline D3D12_DESCRIPTOR_RANGE1 Create(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, uint32_t numDescriptors, uint32_t shaderRegister, 
			uint32_t space = 0, D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE, uint32_t offset = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND)
		{
			return D3D12_DESCRIPTOR_RANGE1{ rangeType, numDescriptors, shaderRegister, space, flags, offset };
		}
	};

	/* Factory helper struct that constructs D3D12_ROOT_PARAMETER1s... */
	/* [Constants, CBVs, SRVs, UAVs, RootDescTables] */
	struct D3D12RootParameter final
	{
		/* Creates a RootParameter as 32-bit constants... */
		constexpr static inline D3D12_ROOT_PARAMETER1 AsConstants(uint32_t numConstants, D3D12_SHADER_VISIBILITY visibility, uint32_t shaderRegister, uint32_t space = 0)
		{
			D3D12_ROOT_PARAMETER1 param{};
			param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS; // Float constants
			param.ShaderVisibility = visibility;
			param.Constants.Num32BitValues = numConstants;
			param.Constants.ShaderRegister = shaderRegister;
			param.Constants.RegisterSpace = space;

			return param;
		}

		/* Creates a RootParameter as a Constant Buffer View... */
		constexpr static inline D3D12_ROOT_PARAMETER1 AsCBV(D3D12_SHADER_VISIBILITY visibility, 
			uint32_t shaderRegister, uint32_t space = 0, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
		{
			return AsDescriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, visibility, shaderRegister, space, flags);
		}

		/* Creates a RootParameter as a Shader Resource View... */
		constexpr static inline D3D12_ROOT_PARAMETER1 AsSRV(D3D12_SHADER_VISIBILITY visibility,
			uint32_t shaderRegister, uint32_t space = 0, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
		{
			return AsDescriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, visibility, shaderRegister, space, flags);
		}

		/* Creates a RootParameter as a Unordered Access View... */
		constexpr static inline D3D12_ROOT_PARAMETER1 AsUAV(D3D12_SHADER_VISIBILITY visibility,
			uint32_t shaderRegister, uint32_t space = 0, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
		{
			return AsDescriptor(D3D12_ROOT_PARAMETER_TYPE_UAV, visibility, shaderRegister, space, flags);
		}

		/* Creates a RootParameter as a Root Descriptor-Table... */
		constexpr static inline D3D12_ROOT_PARAMETER1 AsRootDescTable(D3D12_SHADER_VISIBILITY visibility,
			const D3D12_DESCRIPTOR_RANGE1* ranges, uint32_t numRanges)
		{
			D3D12_ROOT_PARAMETER1 param{};
			param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			param.ShaderVisibility = visibility;
			param.DescriptorTable.NumDescriptorRanges = numRanges;
			param.DescriptorTable.pDescriptorRanges = ranges;
			return param;
		}

	private:
		/* SRVs, UAVs & CBVs... */
		constexpr static inline D3D12_ROOT_PARAMETER1 AsDescriptor(D3D12_ROOT_PARAMETER_TYPE type, D3D12_SHADER_VISIBILITY visibility,
			uint32_t shaderRegister, uint32_t space = 0, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
		{
			D3D12_ROOT_PARAMETER1 param{};
			param.ParameterType = type;
			param.ShaderVisibility = visibility;
			param.Descriptor.ShaderRegister = shaderRegister;
			param.Descriptor.RegisterSpace = space;
			param.Descriptor.Flags = flags;
			return param;
		}
	};

	struct D3D12RootSignatureDesc;

	/* Wrapper around ID3D12RootSignature */
	class D3D12RootSignature final
	{
	public:
		static Ptr<D3D12RootSignature> Create(const Ptr<D3D12API> api, const D3D12RootSignatureDesc& desc);

		Ptr<ID3D12RootSignature> GetD3D12RootSignature() const;

	private:
		D3D12RootSignature() = default;

		ID3D12RootSignature* mpD3D12RootSignature;
	};

	/* Helper struct to construct a D3D12RootSignature */
	struct D3D12RootSignatureDesc final
	{
		constexpr static D3D12_ROOT_SIGNATURE_FLAGS DefaultFlags =
			D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;

		Vector<D3D12_ROOT_PARAMETER1> Parameters{};
		Vector<D3D12_STATIC_SAMPLER_DESC> StaticSamplers{};
		D3D12_ROOT_SIGNATURE_FLAGS Flags = DefaultFlags;
	};
}


