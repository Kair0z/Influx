#pragma once

#include "InfluxGraphics/RHITypes.h"
#include "D3D12.h"

namespace Influx::Graphics::Conversion
{
	constexpr D3D12_COMMAND_LIST_TYPE ToDx12(ERHICommandQueueType type)
	{
		switch (type)
		{
		default:
		case ERHICommandQueueType::Graphics:
			return D3D12_COMMAND_LIST_TYPE_DIRECT;
		}
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
		case ERHIResourceViewType::Invalid: return D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
		}
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
		default: return D3D12_RESOURCE_STATE_COMMON;
		}
	}

	constexpr DXGI_FORMAT ToDx12(ERHIFormat format)
	{
		switch (format) {
		case ERHIFormat::D_32_Float: return DXGI_FORMAT_D32_FLOAT;
		case ERHIFormat::RGBA_32_Float: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case ERHIFormat::RGBA_8_Unorm: return DXGI_FORMAT_R8G8B8A8_UNORM;
		default:
		case ERHIFormat::INVALID: return DXGI_FORMAT_UNKNOWN;
		}
	}

	constexpr D3D12_PRIMITIVE_TOPOLOGY ToDx12(const ERHIPrimitiveTopology topology)
	{
		switch (topology)
		{
		default:
		case ERHIPrimitiveTopology::TriangleList: return D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		}
	}

	constexpr D3D12_CULL_MODE ToDx12(const ERHICullMode cullMode)
	{
		switch (cullMode)
		{
		default:
		case ERHICullMode::None: return D3D12_CULL_MODE_NONE;
		case ERHICullMode::BackFaceCull: return D3D12_CULL_MODE_BACK;
		case ERHICullMode::FrontFaceCull: return D3D12_CULL_MODE_FRONT;
		}
	}

	constexpr D3D12_FILL_MODE ToDx12(const ERHIFillMode fillMode)
	{
		switch (fillMode)
		{
		default:
		case ERHIFillMode::Solid: return D3D12_FILL_MODE_SOLID;
		case ERHIFillMode::Wireframe: return D3D12_FILL_MODE_WIREFRAME;
		}
	}

	constexpr D3D12_PRIMITIVE_TOPOLOGY_TYPE ToDx12(const ERHIPrimitiveTopologyType topologyType)
	{
		switch (topologyType)
		{
		default:
		case ERHIPrimitiveTopologyType::Triangle: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		}
	}

	constexpr D3D12_ROOT_PARAMETER_TYPE ToDx12(const ERHIResourceBindingType bindingType)
	{
		switch (bindingType)
		{
		default:
		case ERHIResourceBindingType::Constants: return D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		case ERHIResourceBindingType::CBV:			return D3D12_ROOT_PARAMETER_TYPE_CBV;
		case ERHIResourceBindingType::SRV:			return D3D12_ROOT_PARAMETER_TYPE_SRV;
		case ERHIResourceBindingType::UAV:			return D3D12_ROOT_PARAMETER_TYPE_UAV;
		}
	}
}