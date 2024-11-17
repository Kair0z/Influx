#pragma once

#if _DLL
	#define INFLUX_RENDER_API __declspec(dllexport)
#else
	#define INFLUX_RENDER_API __declspec(dllimport)
#endif

// Imgui
struct ImDrawData;

// influx::core
#include "core/basetypes.h"
#include "core/function.h"
#include "core/container/vector.h"
#include "core/string.h"
#include "core/math/vector.h"
#include "core/shader.h"

// influx::platform
#include "influx_platform/window.h"

// influx::renderer
#include "influx_renderer/types.h"
#include "influx_renderer/constants.h"
#include "influx_renderer/target.h"
#include "influx_renderer/depth_stencil.h"
#include "influx_renderer/mesh.h"
#include "influx_renderer/texture.h"
#include "influx_renderer/material.h"
#include "influx_renderer/scene.h"
#include "influx_renderer/stats.h"

// influx::shader
#include "influx_shader.h"

namespace influx::renderer
{
	// shader data
	struct shader_data final
	{
		shader::e_shader_type m_type;
		shader::reflection m_reflection;
		vector<byte> m_bytecode;
	};

	// arguments to pass to backend initialization
	struct init_args final
	{
		e_render_api m_api_type = e_render_api::dx12;
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

	// targets
	INFLUX_RENDER_API target* create_target(const target_create_args& args);

	// creates / switches to the appropriate target representation of our window backbuffer
	INFLUX_RENDER_API target* get_window_target(const platform::window& window);


	// 1. acquire the frame to render
	INFLUX_RENDER_API void acquire_swapchain_frame();

	// 2. draw the scene to window / intermediate target
	INFLUX_RENDER_API void draw_scene(const scene& scene, const target& target);

	// - imgui rendering
	INFLUX_RENDER_API void draw_imgui(ImDrawData* draw_data, const target& target);

	// - sprite rendering
	INFLUX_RENDER_API void draw_2D(const scene2D& scene, const target& target);

	// 3. (optional) copy intermediate data
	INFLUX_RENDER_API void copy_target(const target& source, const target& dest);

	// 4. present to window swapchain
	INFLUX_RENDER_API void present_swapchain(const present_args& args);

	INFLUX_RENDER_API void wait_gpu_finished();

	// loading assets into the renderer
	INFLUX_RENDER_API void load(const string& title, const mesh_data& data);

	INFLUX_RENDER_API void load(const string& title, const texture_data& data);

	INFLUX_RENDER_API void load(const string& title, const shader_data& data);

	INFLUX_RENDER_API void load(const string& title, const material& data);

	INFLUX_RENDER_API bool has_mesh(const string& title);

	INFLUX_RENDER_API bool has_texture(const string& title);

	INFLUX_RENDER_API bool has_shader(const string& title);

	INFLUX_RENDER_API bool has_material(const string& title);

	// graphics info
	struct memory_info final
	{
		size_t m_gpu_usage = 0u;
		size_t m_gpu_budget = 0u;
	};

	INFLUX_RENDER_API memory_info get_memory_info();
}