#pragma once

#ifndef __INFLUX_RENDERER_H_
#define __INFLUX_RENDERER_H_

#if _DLL
	#define INFLUX_RENDER_API __declspec(dllexport)
#else
	#define INFLUX_RENDER_API __declspec(dllimport)
#endif

#include "Core/basetypes.h"
#include "Core/Function.h"
#include "Core/Platform/Platform.h"

namespace influx::renderer
{
#pragma region konstants
	constexpr static bool	k_useWarp = true;
	constexpr static uint8	k_max_srvs = 64u;
#pragma endregion

	enum class INFLUX_RENDER_API e_buffering
	{
		dubble = 2,
		tripple = 3,
		max
	};

	enum class INFLUX_RENDER_API e_render_api
	{
		dx12,

		unsupported, // everything below is unsupported!
		vulkan,
		rhi,
		max
	};

	struct INFLUX_RENDER_API init_args final
	{
		e_render_api m_api_type = e_render_api::dx12;
	};

	struct INFLUX_RENDER_API present_args final
	{
		bool m_vsync = true;
	};

	class INFLUX_RENDER_API command_list final
	{
	public:
		void foo()
		{

		}

	private:
		int i = 0;
	};


	INFLUX_RENDER_API void initialize(const init_args& args);

	INFLUX_RENDER_API command_list* record();

	INFLUX_RENDER_API void submit(const command_list* list);

	INFLUX_RENDER_API void submit(const vector<command_list*>& lists);

	INFLUX_RENDER_API void present_to_window(platform::window_handle window_handle, const present_args& args);

	INFLUX_RENDER_API bool is_initialized();

	INFLUX_RENDER_API void cleanup();
}

#endif