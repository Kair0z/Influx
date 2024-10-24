#pragma once
#include "d3d12_headers.h"

namespace influx::rhi::dx12
{
    inline D3D12_COMMAND_LIST_TYPE convert(e_queue_type type)
	{
		switch (type)
		{
		case e_queue_type::graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
		case e_queue_type::compute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
		case e_queue_type::copy: return D3D12_COMMAND_LIST_TYPE_COPY;
		default: return D3D12_COMMAND_LIST_TYPE_DIRECT;
		}
	}

	inline DXGI_FORMAT convert(e_format format)
	{
		switch (format)
		{
		case e_format::rgba8: return DXGI_FORMAT_R8G8B8A8_UNORM;
		case e_format::r32: return DXGI_FORMAT_R32_FLOAT;
		case e_format::rg32: return DXGI_FORMAT_R32G32_FLOAT;
		case e_format::rgb32: return DXGI_FORMAT_R32G32B32_FLOAT;
		case e_format::rgba32: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case e_format::d32: return DXGI_FORMAT_D32_FLOAT;
		case e_format::u16: return DXGI_FORMAT_R16_UINT;
		case e_format::u32: return DXGI_FORMAT_R32_UINT;
		default: 
			influx_assert(false);
			return DXGI_FORMAT_R8G8_UNORM;
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
		case e_descriptor_heap_type::srv: return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
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
		case e_resource_state::depth_write: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
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
		case e_comparison_func::always: return D3D12_COMPARISON_FUNC_ALWAYS;
		case e_comparison_func::less: return D3D12_COMPARISON_FUNC_LESS;
		case e_comparison_func::gequal: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		case e_comparison_func::greater: return D3D12_COMPARISON_FUNC_GREATER;
		default:
		case e_comparison_func::count: 
			influx_assert(false);
			return D3D12_COMPARISON_FUNC_LESS_EQUAL;
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
		case e_heap_type::count:
			influx_assert(false);
			return D3D12_HEAP_TYPE_DEFAULT;
		}
	}

	inline D3D12_SHADER_VISIBILITY convert(e_shader_visibility vis)
	{
		switch (vis)
		{
		case e_shader_visibility::all: return D3D12_SHADER_VISIBILITY_ALL;
		case e_shader_visibility::vertex: return D3D12_SHADER_VISIBILITY_VERTEX;
		case e_shader_visibility::pixel: return D3D12_SHADER_VISIBILITY_PIXEL;
		case e_shader_visibility::domain: return D3D12_SHADER_VISIBILITY_DOMAIN;
		case e_shader_visibility::hull: return D3D12_SHADER_VISIBILITY_HULL;
		case e_shader_visibility::geometry: return D3D12_SHADER_VISIBILITY_GEOMETRY;
		default:
		case e_shader_visibility::count:
			influx_assert(false);
			return D3D12_SHADER_VISIBILITY_ALL;
		}
	}

	inline D3D12_TEXTURE_ADDRESS_MODE convert(e_texture_wrap_mode wrap)
	{
		switch (wrap)
		{
		case e_texture_wrap_mode::wrap: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		case e_texture_wrap_mode::border: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		case e_texture_wrap_mode::mirror: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		case e_texture_wrap_mode::mirror_once: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
		case e_texture_wrap_mode::clamp: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		default:
		case e_texture_wrap_mode::count:
			influx_assert(false);
			return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		}
	}

	inline D3D12_STATIC_BORDER_COLOR convert(e_border_color color)
	{
		switch (color)
		{
		case e_border_color::white: return D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		case e_border_color::black: return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		case e_border_color::black_transparent: return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		default:
		case e_border_color::count:
			return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		}
	}

	inline D3D12_FILTER convert(e_filter filter)
	{
		return (D3D12_FILTER)filter;
	}

	inline D3D12_BLEND convert(e_blend blend)
	{
		return (D3D12_BLEND)blend;
	}

	inline D3D12_BLEND_OP convert(e_blendop op)
	{
		return (D3D12_BLEND_OP)op;
	}

	inline D3D12_FILL_MODE convert(e_fill_mode mode)
	{
		switch (mode)
		{
		case e_fill_mode::wireframe: return D3D12_FILL_MODE_WIREFRAME;
		case e_fill_mode::solid: return D3D12_FILL_MODE_SOLID;
		default:
		case e_fill_mode::count:
			influx_assert(false);
			return  D3D12_FILL_MODE_SOLID;
		}
	}

	constexpr D3D12_RENDER_PASS_FLAGS translate(e_renderpass_flags flags)
	{
		D3D12_RENDER_PASS_FLAGS translated = D3D12_RENDER_PASS_FLAG_NONE;
		if (flags & e_renderpass_flags::read_only_depth) translated |= D3D12_RENDER_PASS_FLAG_BIND_READ_ONLY_DEPTH;
		if (flags & e_renderpass_flags::read_only_stencil) translated |= D3D12_RENDER_PASS_FLAG_BIND_READ_ONLY_STENCIL;
		if (flags & e_renderpass_flags::allow_uav_write) translated |= D3D12_RENDER_PASS_FLAG_ALLOW_UAV_WRITES;
		if (flags & e_renderpass_flags::suspending) translated |= D3D12_RENDER_PASS_FLAG_SUSPENDING_PASS;
		if (flags & e_renderpass_flags::resuming) translated |= D3D12_RENDER_PASS_FLAG_RESUMING_PASS;
		return translated;
	}

	constexpr D3D12_RENDER_PASS_BEGINNING_ACCESS translate(e_load_op load_op)
	{
		D3D12_RENDER_PASS_BEGINNING_ACCESS access{};
		switch (load_op)
		{
		case e_load_op::discard: access.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD; break;
		case e_load_op::preserve: access.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE; break;
		case e_load_op::clear: access.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR; break;
		case e_load_op::no_access: access.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS; break;
		}
		return access;
	}

	constexpr D3D12_RENDER_PASS_ENDING_ACCESS translate(e_store_op store_op)
	{
		D3D12_RENDER_PASS_ENDING_ACCESS access{};
		switch (store_op)
		{
		case e_store_op::discard: access.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD; break;
		case e_store_op::preserve: access.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE; break;
		case e_store_op::resolve: access.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_RESOLVE; break;
		case e_store_op::no_access: access.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS; break;
		}
		return access;
	}
}