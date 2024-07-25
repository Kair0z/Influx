#pragma once

#include "influx_graphics/common.h"
#include "influx_graphics/commandqueue.h"
#include "influx_graphics/resource.h"
#include "influx_graphics/descriptorheap.h"

// dx12 includes
#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include "d3dx12.h"

namespace influx::graphics
{
	inline D3D12_COMMAND_LIST_TYPE convert(e_command_queue_type type)
	{
		switch (type)
		{
		case e_command_queue_type::graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
		case e_command_queue_type::compute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
		case e_command_queue_type::copy: return D3D12_COMMAND_LIST_TYPE_COPY;
		default: return D3D12_COMMAND_LIST_TYPE_DIRECT;
		}
	}

	inline DXGI_FORMAT convert(e_format format)
	{
		switch (format)
		{
		case e_format::rgba8: return DXGI_FORMAT_R8G8B8A8_UNORM;
		case e_format::d32: return DXGI_FORMAT_D32_FLOAT;
		case e_format::u32: return DXGI_FORMAT_R32_UINT;
		default: return DXGI_FORMAT_R8G8_UNORM;
		}
	}

	inline D3D12_RESOURCE_FLAGS convert(e_resource_flags flags)
	{
		switch (flags)
		{
		case e_resource_flags::none: return D3D12_RESOURCE_FLAG_NONE;
		case e_resource_flags::depth_stencil: return D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		case e_resource_flags::render_target: return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		default: return D3D12_RESOURCE_FLAG_NONE;
		}
	}

	inline D3D12_DESCRIPTOR_HEAP_TYPE convert(e_descriptor_heap_type type)
	{
		switch (type)
		{
		case e_descriptor_heap_type::rtv: return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		case e_descriptor_heap_type::dsv: return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		case e_descriptor_heap_type::cbv: return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		case e_descriptor_heap_type::sampler: return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		default:
		case e_descriptor_heap_type::count:	return D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
		}
	}

	inline D3D12_RESOURCE_STATES convert(e_resource_state state)
	{
		switch (state)
		{
		case e_resource_state::common: return D3D12_RESOURCE_STATE_COMMON;
		case e_resource_state::copy_dest: return D3D12_RESOURCE_STATE_COPY_DEST;
		case e_resource_state::copy_source: return D3D12_RESOURCE_STATE_COPY_SOURCE;
		case e_resource_state::render_target: return D3D12_RESOURCE_STATE_RENDER_TARGET;
		case e_resource_state::present: return D3D12_RESOURCE_STATE_PRESENT;
		case e_resource_state::read:	return D3D12_RESOURCE_STATE_GENERIC_READ;
		case e_resource_state::shader_resource: return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		default:
		case e_resource_state::count:	return D3D12_RESOURCE_STATE_COMMON;
		}
	}

	inline D3D12_COMPARISON_FUNC convert(e_comparison_func func)
	{
		switch (func)
		{
		case e_comparison_func::lequal: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		default:
		case e_comparison_func::count: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		}
	}

	inline D3D12_CULL_MODE convert(e_cull_mode mode)
	{
		switch (mode)
		{
		case e_cull_mode::back: return D3D12_CULL_MODE_BACK;
		case e_cull_mode::front: return D3D12_CULL_MODE_FRONT;
		case e_cull_mode::nocull: return D3D12_CULL_MODE_NONE;
		default:
		case e_cull_mode::count: return D3D12_CULL_MODE_NONE;
		}
	}

	inline D3D12_PRIMITIVE_TOPOLOGY_TYPE convert(e_primitive_topology_type type)
	{
		switch (type)
		{
		case e_primitive_topology_type::triangle: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		default:
		case e_primitive_topology_type::count: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
		}
	}

	inline D3D_PRIMITIVE_TOPOLOGY convert(e_primitive_topology topo)
	{
		switch (topo)
		{
		case e_primitive_topology::trilist: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		default:
		case e_primitive_topology::count: return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		}
	}

	inline D3D12_HEAP_TYPE convert(e_heap_type type)
	{
		switch (type)
		{
		case e_heap_type::gpu: return D3D12_HEAP_TYPE_DEFAULT;
		case e_heap_type::shared: return D3D12_HEAP_TYPE_UPLOAD;
		case e_heap_type::readback: return D3D12_HEAP_TYPE_READBACK;
		default:
		case e_heap_type::count: return D3D12_HEAP_TYPE_DEFAULT;
		}
	}
}