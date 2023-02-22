#pragma once

#include "InfluxGraphics/RHITypes.h"
#include "D3D12.h"

namespace Influx::Graphics::Conversion
{
	constexpr D3D12_COMMAND_LIST_TYPE ToDx12(ERHICommandQueueType type)
	{
		switch (type)
		{
		case ERHICommandQueueType::Graphics:	return D3D12_COMMAND_LIST_TYPE_DIRECT;
		case ERHICommandQueueType::Compute:		return D3D12_COMMAND_LIST_TYPE_COMPUTE;
		default:
			assert(false); // Todo...
		}

		return {};
	}

	constexpr D3D12_DESCRIPTOR_HEAP_TYPE ToDx12(ERHIResourceViewType type)
	{
		switch (type)
		{
		case ERHIResourceViewType::DSV: return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		case ERHIResourceViewType::Resource: return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		case ERHIResourceViewType::RTV: return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		case ERHIResourceViewType::Sampler: return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;

		default:
		case ERHIResourceViewType::Invalid:
			assert(false); // Todo...
			return D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
		}

		return {};
	}

	constexpr D3D12_RESOURCE_STATES ToDx12(const ERHIResourceState state)
	{
		switch (state)
		{
		case ERHIResourceState::Common: return D3D12_RESOURCE_STATE_COMMON;
		case ERHIResourceState::VertexAndConstantBuffer: return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		case ERHIResourceState::IndexBuffer: return D3D12_RESOURCE_STATE_INDEX_BUFFER;
		case ERHIResourceState::RenderTarget: return D3D12_RESOURCE_STATE_RENDER_TARGET;
		case ERHIResourceState::UnorderedAccess: return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		case ERHIResourceState::DepthWrite: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
		case ERHIResourceState::DepthRead: return D3D12_RESOURCE_STATE_DEPTH_READ;
		case ERHIResourceState::Present: return D3D12_RESOURCE_STATE_PRESENT;
		case ERHIResourceState::RaytracingAS: return D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
		case ERHIResourceState::CopyDest: return D3D12_RESOURCE_STATE_COPY_DEST;
		case ERHIResourceState::CopySource: return D3D12_RESOURCE_STATE_COPY_SOURCE;
		case ERHIResourceState::GenericRead: return D3D12_RESOURCE_STATE_GENERIC_READ;
		case ERHIResourceState::NonPixelReadResource: return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		case ERHIResourceState::PixelShaderResource: return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		default:
			assert(false); // Todo...
		}

		return {};
	}

	constexpr DXGI_FORMAT ToDx12(ERHIFormat format)
	{
		switch (format) 
		{
			case ERHIFormat::D_32_Float:	return DXGI_FORMAT_D32_FLOAT;
			case ERHIFormat::RGB_32_Float:	return DXGI_FORMAT_R32G32B32_FLOAT;
			case ERHIFormat::RGBA_32_Float: return DXGI_FORMAT_R32G32B32A32_FLOAT;
			case ERHIFormat::RGBA_8_Unorm:	return DXGI_FORMAT_R8G8B8A8_UNORM;
			case ERHIFormat::Unknown:		return DXGI_FORMAT_UNKNOWN;

			default:
			case ERHIFormat::INVALID: 
				assert(false); 
				return DXGI_FORMAT_UNKNOWN;
		}

		return {};
	}

	constexpr D3D12_PRIMITIVE_TOPOLOGY ToDx12(const ERHIPrimitiveTopology topology)
	{
		switch (topology)
		{
		case ERHIPrimitiveTopology::TriangleList: return D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		default:
			assert(false); // Todo...
		}

		return {};
	}

	constexpr D3D12_CULL_MODE ToDx12(const ERHICullMode cullMode)
	{
		switch (cullMode)
		{
		case ERHICullMode::None: return D3D12_CULL_MODE_NONE;
		case ERHICullMode::BackFaceCull: return D3D12_CULL_MODE_BACK;
		case ERHICullMode::FrontFaceCull: return D3D12_CULL_MODE_FRONT;
		default:
			assert(false); // Todo...
		}

		return {};
	}

	constexpr D3D12_FILL_MODE ToDx12(const ERHIFillMode fillMode)
	{
		switch (fillMode)
		{
		case ERHIFillMode::Solid: return D3D12_FILL_MODE_SOLID;
		case ERHIFillMode::Wireframe: return D3D12_FILL_MODE_WIREFRAME;
		default:
			assert(false); // Todo...
		}

		return {};
	}

	constexpr D3D12_PRIMITIVE_TOPOLOGY_TYPE ToDx12(const ERHIPrimitiveTopologyType topologyType)
	{
		switch (topologyType)
		{
		case ERHIPrimitiveTopologyType::Triangle: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		default:
			assert(false); // Todo...
		}

		return {};
	}

	constexpr D3D12_ROOT_PARAMETER_TYPE ToDx12(const ERHIResourceBindingType bindingType)
	{
		switch (bindingType)
		{
		case ERHIResourceBindingType::Constants: return D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		case ERHIResourceBindingType::CBV:			return D3D12_ROOT_PARAMETER_TYPE_CBV;
		case ERHIResourceBindingType::SRV:			return D3D12_ROOT_PARAMETER_TYPE_SRV;
		case ERHIResourceBindingType::UAV:			return D3D12_ROOT_PARAMETER_TYPE_UAV;
		default:
			assert(false); // Todo...
		}

		return {};
	}

	constexpr D3D12_BLEND ToDx12(const ERHIBlend blend)
	{
		switch (blend)
		{
			case ERHIBlend::Zero			: return D3D12_BLEND_ZERO;
			case ERHIBlend::One				: return D3D12_BLEND_ONE;
			case ERHIBlend::SrcColour		: return D3D12_BLEND_SRC_COLOR;
			case ERHIBlend::InvSrcColour	: return D3D12_BLEND_INV_SRC_COLOR;
			case ERHIBlend::SrcAlpha		: return D3D12_BLEND_SRC_ALPHA;
			case ERHIBlend::InvSrcAlpha		: return D3D12_BLEND_INV_SRC_ALPHA;
			case ERHIBlend::DestAlpha		: return D3D12_BLEND_DEST_ALPHA;
			case ERHIBlend::InvDestAlpha	: return D3D12_BLEND_INV_DEST_ALPHA;
			case ERHIBlend::DestColour		: return D3D12_BLEND_DEST_COLOR;
			case ERHIBlend::InvDestColour	: return D3D12_BLEND_INV_DEST_COLOR;
			case ERHIBlend::SrcAlphaSat		: return D3D12_BLEND_SRC_ALPHA_SAT;
			case ERHIBlend::BlendFactor		: return D3D12_BLEND_BLEND_FACTOR;
			case ERHIBlend::InvBlendFactor	: return D3D12_BLEND_INV_BLEND_FACTOR;
			case ERHIBlend::Src1Colour		: return D3D12_BLEND_SRC1_COLOR;
			case ERHIBlend::InvSrc1Colour	: return D3D12_BLEND_INV_SRC1_COLOR;
			case ERHIBlend::Src1Alpha		: return D3D12_BLEND_SRC1_ALPHA;
			case ERHIBlend::InvSrc1Alpha	: return D3D12_BLEND_INV_SRC1_ALPHA;
		default:
			assert(false); // Todo...
		}

		return {};
	}

	constexpr D3D12_BLEND_OP ToDx12(const ERHIBlendOperation blendOp)
	{
		switch (blendOp)
		{
			case ERHIBlendOperation::OpAdd		: return D3D12_BLEND_OP_ADD;
			case ERHIBlendOperation::OpSub		: return D3D12_BLEND_OP_SUBTRACT;
			case ERHIBlendOperation::OpRevSub	: return D3D12_BLEND_OP_REV_SUBTRACT;
			case ERHIBlendOperation::OpMin		: return D3D12_BLEND_OP_MIN;
			case ERHIBlendOperation::OpMax		: return D3D12_BLEND_OP_MAX;
		default:
			assert(false); // Todo...
		}

		return {};
	}

	constexpr D3D12_LOGIC_OP ToDx12(const ERHILogicOperation logicOp)
	{
		switch (logicOp)
		{
			case ERHILogicOperation::Clear		: return D3D12_LOGIC_OP_CLEAR;
			case ERHILogicOperation::Set		: return D3D12_LOGIC_OP_SET;
			case ERHILogicOperation::Copy		: return D3D12_LOGIC_OP_COPY;
			case ERHILogicOperation::CopyInv 	: return D3D12_LOGIC_OP_COPY_INVERTED;
			case ERHILogicOperation::NoOp		: return D3D12_LOGIC_OP_NOOP;
			case ERHILogicOperation::Invert		: return D3D12_LOGIC_OP_INVERT;
			case ERHILogicOperation::And		: return D3D12_LOGIC_OP_AND;
			case ERHILogicOperation::Nand		: return D3D12_LOGIC_OP_NAND;
			case ERHILogicOperation::Or			: return D3D12_LOGIC_OP_OR;
			case ERHILogicOperation::Nor		: return D3D12_LOGIC_OP_NOR;
			case ERHILogicOperation::Xor		: return D3D12_LOGIC_OP_XOR;
			case ERHILogicOperation::Equiv		: return D3D12_LOGIC_OP_EQUIV;
			case ERHILogicOperation::RevAnd		: return D3D12_LOGIC_OP_AND_REVERSE;
			case ERHILogicOperation::InvAnd		: return D3D12_LOGIC_OP_AND_INVERTED;
			case ERHILogicOperation::RevOr		: return D3D12_LOGIC_OP_OR_REVERSE;
			case ERHILogicOperation::InvOr		: return D3D12_LOGIC_OP_OR_INVERTED;
		default:
			assert(false); // Todo...
		}

		return {};
	}
}