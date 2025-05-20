#pragma once

#if _DLL
#define INFLUX_RENDER_API __declspec(dllexport)
#else
#define INFLUX_RENDER_API __declspec(dllimport)
#endif

// influx::core
#include "core/basetypes.h"
#include "core/result.h"

namespace influx::renderer
{
	// the result type
	template <typename _t = char>
	using result = influx::result<_t, const char*>;

	// imgui type
	class imgui_texid_provider
	{
	public:
		virtual void* get_tex_descriptor() const = 0;
		virtual void* get_tex_resource() const = 0;
		virtual debug_name get_rendergraph_id() const = 0;
	};
	using imgui_tex_id = imgui_texid_provider*;

	// num backbuffers held by a swapchain
	enum class e_buffering : uint8
	{
		dubble = 2,
		tripple = 3,
		max
	};

	constexpr static uint8 get_num_buffers(e_buffering buffering)
	{
		switch (buffering)
		{
		case e_buffering::dubble: return 2u;
		case e_buffering::tripple: return 3u;
		default:
		case e_buffering::max: return (uint8)-1;
		}
	}

	// graphics api
	enum class e_render_api : uint8
	{
		dx12,
		vulkan,
		unsupported,
		max
	};
}