#pragma once

// influx::graphics
#include "influx_graphics/common.h"
#include "influx_graphics/queue.h"
#include "influx_graphics/resource.h"
#include "influx_graphics/descriptors.h"
#include "influx_graphics/renderpass.h"
#include "influx_graphics/d3d12/dx12_headers.h"

namespace influx::graphics
{
	inline D3D12_COMMAND_LIST_TYPE translate(e_queue_type type)
	{
		switch (type)
		{
		case e_queue_type::graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
		case e_queue_type::compute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
		case e_queue_type::copy: return D3D12_COMMAND_LIST_TYPE_COPY;
		default: return D3D12_COMMAND_LIST_TYPE_DIRECT;
		}
	}

	inline D3D12_SRV_DIMENSION translate(resource::e_type type)
	{
		switch (type)
		{
		case resource::e_type::buffer: return D3D12_SRV_DIMENSION_BUFFER;
		case resource::e_type::tex2D: return D3D12_SRV_DIMENSION_TEXTURE2D;
		case resource::e_type::tex3D: return D3D12_SRV_DIMENSION_TEXTURE3D;
		case resource::e_type::cubemap: return D3D12_SRV_DIMENSION_TEXTURECUBE;
		}
	}

	inline e_format translate(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM:		return e_format::rgba8;
		case DXGI_FORMAT_R32_FLOAT:				return e_format::r32;
		case DXGI_FORMAT_R32G32_FLOAT:			return e_format::rg32;
		case DXGI_FORMAT_R32G32B32_FLOAT:		return e_format::rgb32;
		case DXGI_FORMAT_R32G32B32A32_FLOAT:	return e_format::rgba32;
		case DXGI_FORMAT_D32_FLOAT:				return e_format::d32;
		case DXGI_FORMAT_R16_UINT:				return e_format::u16;
		case DXGI_FORMAT_R32_UINT:				return e_format::u32;
		case DXGI_FORMAT_R32G32B32A32_UINT:		return e_format::rgba_u32;
		default:
			influx_assert(false);
			return e_format::count;
		}
	}

	inline DXGI_FORMAT translate(e_format format)
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
		case e_format::rgba_u32: return DXGI_FORMAT_R32G32B32A32_UINT;
		default: 
			influx_assert(false);
			return DXGI_FORMAT_R8G8_UNORM;
		}
	}

	inline D3D12_RESOURCE_FLAGS translate(e_bind_flags flags)
	{
		D3D12_RESOURCE_FLAGS result = D3D12_RESOURCE_FLAG_NONE;
		if (has_flag(flags, e_bind_flags::dsv)) result |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		if (has_flag(flags, e_bind_flags::rtv)) result |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		if (has_flag(flags, e_bind_flags::srv)) result |= D3D12_RESOURCE_FLAG_NONE;
		if (has_flag(flags, e_bind_flags::uav)) result |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		return result;
	}

	inline D3D12_DESCRIPTOR_HEAP_TYPE translate(e_descriptor_heap_type type)
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

	inline D3D12_RESOURCE_STATES translate(e_resource_state state)
	{
		D3D12_RESOURCE_STATES result = D3D12_RESOURCE_STATE_COMMON;
		if (has_flag(state, e_resource_state::present			)) result |= D3D12_RESOURCE_STATE_PRESENT;
		if (has_flag(state, e_resource_state::render_target		)) result |= D3D12_RESOURCE_STATE_RENDER_TARGET;
		if (has_flag(state, e_resource_state::depth_target		)) result |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
		if (has_flag(state, e_resource_state::depth_readonly	)) result |= D3D12_RESOURCE_STATE_DEPTH_READ;
		if (has_flag(state, e_resource_state::vs_srv			)) result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		if (has_flag(state, e_resource_state::ps_srv			)) result |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		if (has_flag(state, e_resource_state::cs_srv			)) result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		if (has_flag(state, e_resource_state::vs_uav			)) result |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		if (has_flag(state, e_resource_state::ps_uav			)) result |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		if (has_flag(state, e_resource_state::cs_uav			)) result |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		if (has_flag(state, e_resource_state::clear_uav			)) result |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		if (has_flag(state, e_resource_state::copy_src			)) result |= D3D12_RESOURCE_STATE_COPY_SOURCE;
		if (has_flag(state, e_resource_state::copy_dst			)) result |= D3D12_RESOURCE_STATE_COPY_DEST;
		if (has_flag(state, e_resource_state::shading_rate		)) result |= D3D12_RESOURCE_STATE_COMMON;
		if (has_flag(state, e_resource_state::indexbuffer		)) result |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
		if (has_flag(state, e_resource_state::indirect_args		)) result |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
		if (has_flag(state, e_resource_state::as_read			)) result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
		if (has_flag(state, e_resource_state::as_write			)) result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
		if (has_flag(state, e_resource_state::discard			)) result |= D3D12_RESOURCE_STATE_COMMON;
		if (has_flag(state, e_resource_state::resolve_dst		)) result |= D3D12_RESOURCE_STATE_RESOLVE_DEST;
		if (has_flag(state, e_resource_state::resolve_src		)) result |= D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
		return result;
	}

	inline D3D12_COMPARISON_FUNC translate(e_comparison_func func)
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

	inline D3D12_CULL_MODE translate(e_cull_mode mode)
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

	inline D3D12_PRIMITIVE_TOPOLOGY_TYPE translate(e_primitive_topology_type type)
	{
		switch (type)
		{
		case e_primitive_topology_type::triangle:	return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		case e_primitive_topology_type::line:		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		case e_primitive_topology_type::patch:		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
		case e_primitive_topology_type::point:		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		default:
		case e_primitive_topology_type::count: 
			influx_assert(false);
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
		}
	}

	inline D3D_PRIMITIVE_TOPOLOGY translate(e_primitive_topology topo)
	{
		switch (topo)
		{
		case e_primitive_topology::trilist: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case e_primitive_topology::linelist: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		default:
		case e_primitive_topology::count: 
			influx_assert(false);
			return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		}
	}

	inline D3D12_HEAP_TYPE translate(e_heap_type type)
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

	inline D3D12_SHADER_VISIBILITY translate(e_shader_visibility vis)
	{
		D3D12_SHADER_VISIBILITY result{};

		switch (vis)
		{
		case e_shader_visibility::all: return D3D12_SHADER_VISIBILITY_ALL;
		case e_shader_visibility::vertex: return D3D12_SHADER_VISIBILITY_VERTEX;
		case e_shader_visibility::pixel: return D3D12_SHADER_VISIBILITY_PIXEL;
		case e_shader_visibility::domain: return D3D12_SHADER_VISIBILITY_DOMAIN;
		case e_shader_visibility::hull: return D3D12_SHADER_VISIBILITY_HULL;
		case e_shader_visibility::geometry: return D3D12_SHADER_VISIBILITY_GEOMETRY;
		case e_shader_visibility::compute:	return D3D12_SHADER_VISIBILITY_ALL;

		default: 
			influx_assert(false);
			return D3D12_SHADER_VISIBILITY_ALL;
		}
	}

	inline D3D12_TEXTURE_ADDRESS_MODE translate(e_texture_wrap_mode wrap)
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

	inline D3D12_STATIC_BORDER_COLOR translate(e_border_color color)
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

	inline D3D12_FILTER translate(e_filter filter)
	{
		return (D3D12_FILTER)filter;
	}

	inline D3D12_BLEND translate(e_blend blend)
	{
		return (D3D12_BLEND)blend;
	}

	inline D3D12_BLEND_OP translate(e_blendop op)
	{
		return (D3D12_BLEND_OP)op;
	}

	inline D3D12_FILL_MODE translate(e_fill_mode mode)
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

	constexpr D3D12_COMMAND_LIST_TYPE translate(e_commandlist_type type)
	{
		switch (type)
		{
		case e_commandlist_type::graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
		case e_commandlist_type::compute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
		default:
			influx_assert(false);
			return D3D12_COMMAND_LIST_TYPE_NONE;
		}
	}

	constexpr D3D12_BARRIER_SYNC get_barrier_sync(e_resource_state state)
	{
		D3D12_BARRIER_SYNC sync = D3D12_BARRIER_SYNC_NONE;
		bool is_discard = has_flag(state, e_resource_state::discard);
		if (!is_discard && has_flag(state, e_resource_state::clear_uav)) sync |= D3D12_BARRIER_SYNC_CLEAR_UNORDERED_ACCESS_VIEW;

		if (has_flag(state, e_resource_state::present)) sync |= D3D12_BARRIER_SYNC_ALL;
		if (has_flag(state, e_resource_state::common)) sync |= D3D12_BARRIER_SYNC_ALL;
		if (has_flag(state, e_resource_state::render_target)) sync |= D3D12_BARRIER_SYNC_RENDER_TARGET;
		if (has_any_flag(state, e_resource_state::all_depth)) sync |= D3D12_BARRIER_SYNC_DEPTH_STENCIL;
		if (has_any_flag(state, e_resource_state::all_vs)) sync |= D3D12_BARRIER_SYNC_VERTEX_SHADING;
		if (has_any_flag(state, e_resource_state::all_ps)) sync |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
		if (has_any_flag(state, e_resource_state::all_cs)) sync |= D3D12_BARRIER_SYNC_COMPUTE_SHADING;
		if (has_any_flag(state, e_resource_state::all_copy)) sync |= D3D12_BARRIER_SYNC_COPY;
		if (has_flag(state, e_resource_state::shading_rate)) sync |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
		if (has_flag(state, e_resource_state::indexbuffer)) sync |= D3D12_BARRIER_SYNC_INDEX_INPUT;
		if (has_flag(state, e_resource_state::indirect_args)) sync |= D3D12_BARRIER_SYNC_EXECUTE_INDIRECT;
		if (has_any_flag(state, e_resource_state::all_as)) sync |= D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE;

		return sync;
	}

	constexpr D3D12_BARRIER_LAYOUT get_barrier_layout(e_resource_state state)
	{
		if (has_flag(state, e_resource_state::copy_src) 
			&& has_any_flag(state, e_resource_state::all_srv) 
			&& !has_flag(state, e_resource_state::depth_readonly))
			return D3D12_BARRIER_LAYOUT_GENERIC_READ;

		if (has_flag(state, e_resource_state::copy_dst)
			&& has_any_flag(state, e_resource_state::all_uav))
			return D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COMMON;

		if (has_flag(state, e_resource_state::depth_readonly)
			&& (has_any_flag(state, e_resource_state::all_srv)
				|| has_flag(state, e_resource_state::copy_src)))
			return D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ;

		if (has_flag(state		, e_resource_state::discard))		return D3D12_BARRIER_LAYOUT_UNDEFINED;
		if (has_flag(state		, e_resource_state::present))		return D3D12_BARRIER_LAYOUT_PRESENT;
		if (has_flag(state		, e_resource_state::render_target))			return D3D12_BARRIER_LAYOUT_RENDER_TARGET;
		if (has_flag(state		, e_resource_state::depth_target))            return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
		if (has_flag(state		, e_resource_state::depth_readonly))   return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
		if (has_any_flag(state	, e_resource_state::all_srv))		return D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
		if (has_any_flag(state	, e_resource_state::all_uav))		return D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
		if (has_flag(state		, e_resource_state::clear_uav))		return D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
		if (has_flag(state		, e_resource_state::copy_dst))		return D3D12_BARRIER_LAYOUT_COPY_DEST;
		if (has_flag(state		, e_resource_state::copy_src))		return D3D12_BARRIER_LAYOUT_COPY_SOURCE;
		if (has_flag(state		, e_resource_state::shading_rate))	return D3D12_BARRIER_LAYOUT_SHADING_RATE_SOURCE;

		return D3D12_BARRIER_LAYOUT_UNDEFINED;
	}

	constexpr D3D12_BARRIER_ACCESS get_barrier_access(e_resource_state state)
	{
		if (has_flag(state, e_resource_state::discard)) return D3D12_BARRIER_ACCESS_NO_ACCESS;

		D3D12_BARRIER_ACCESS access = D3D12_BARRIER_ACCESS_COMMON;
		if (has_flag(state, e_resource_state::render_target))             access |= D3D12_BARRIER_ACCESS_RENDER_TARGET;
		if (has_flag(state, e_resource_state::depth_target))             access |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;
		if (has_flag(state, e_resource_state::depth_readonly))    access |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;
		if (has_any_flag(state, e_resource_state::all_srv))       access |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
		if (has_any_flag(state, e_resource_state::all_uav))       access |= D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
		if (has_flag(state, e_resource_state::clear_uav))        access |= D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
		if (has_flag(state, e_resource_state::copy_dst))         access |= D3D12_BARRIER_ACCESS_COPY_DEST;
		if (has_flag(state, e_resource_state::copy_src))         access |= D3D12_BARRIER_ACCESS_COPY_SOURCE;
		if (has_flag(state, e_resource_state::shading_rate))     access |= D3D12_BARRIER_ACCESS_SHADING_RATE_SOURCE;
		if (has_flag(state, e_resource_state::indexbuffer))     access |= D3D12_BARRIER_ACCESS_INDEX_BUFFER;
		if (has_flag(state, e_resource_state::indirect_args))    access |= D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT;
		if (has_flag(state, e_resource_state::as_read))          access |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ;
		if (has_flag(state, e_resource_state::as_write))         access |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE;

		return access;
	}
}