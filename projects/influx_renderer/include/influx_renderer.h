#pragma once

#if _DLL
	#define INFLUX_RENDER_API __declspec(dllexport)
#else
	#define INFLUX_RENDER_API __declspec(dllimport)
#endif

// core dependencies
#include "core/basetypes.h"
#include "core/function.h"
#include "core/platform/platform.h"
#include "core/container/vector.h"
#include "core/string.h"
#include "core/math/vector.h"

// sub-headers
#include "influx_renderer/types.h"
#include "influx_renderer/constants.h"
#include "influx_renderer/target.h"
#include "influx_renderer/mesh.h"
#include "influx_renderer/texture.h"
#include "influx_renderer/material.h"
#include "influx_renderer/scene.h"
#include "influx_renderer/stats.h"

namespace influx::renderer
{
	// arguments to pass to backend initialization
	struct init_args final
	{
		e_render_api m_api_type = e_render_api::dx12;
		string m_resource_dir = "";
	};

	// arguments passed to swapchain presenting
	struct present_args final
	{
		bool m_vsync = false;
	};
	
	// arguments passed to main render call
	struct render_args final
	{
		math::vectorf4 m_clear_colour = {};
	};

	// initialize renderer first!
	INFLUX_RENDER_API void initialize(const init_args& args);

	INFLUX_RENDER_API bool is_initialized();

	INFLUX_RENDER_API void cleanup();

	// create a target to render to
	INFLUX_RENDER_API target* create_target(const target_create_args& args);

	// creates / switches to the appropriate target representation of our window backbuffer
	INFLUX_RENDER_API target* get_window_target(const platform::window_handle& window);

	// issue commands
	INFLUX_RENDER_API void draw_scene(const scene& scene, const target& target);

	INFLUX_RENDER_API void present_swapchain(const present_args& args);

	// loading assets into the renderer
	INFLUX_RENDER_API void load(const string& title, const mesh_data& data);

	INFLUX_RENDER_API void load(const string& title, const texture_data& data);

	INFLUX_RENDER_API void load(const string& title, const material_data& data);

	INFLUX_RENDER_API const mesh_data* find_mesh_data(const string& title); 

	INFLUX_RENDER_API vector<const mesh_data*> get_all_mesh_datas();


	// backend hooks
	INFLUX_RENDER_API void* get_backend_device();

	INFLUX_RENDER_API void* get_backend_texture_gpu_handle(const string& title);

	// statistics
	INFLUX_RENDER_API vector<frame_stats> get_frame_stats(const uint32 over_num_frames = 1u);
}