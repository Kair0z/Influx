#pragma once

#define INFLUX_RENDER_BINDLESS 1

// Imgui
struct ImDrawData;

// influx::core
#include "core/basetypes.h"
#include "core/function.h"
#include "core/container/vector.h"
#include "core/string.h"
#include "core/math/vector.h"
#include "core/shader.h"
#include "core/material/material.h"
#include "core/time.h"
#include "core/result.h"

// influx::platform
#include "influx_platform/window.h"

// influx::renderer
#include "influx_renderer/types.h"
#include "influx_renderer/constants.h"
#include "influx_renderer/target.h"
#include "influx_renderer/depth_stencil.h"
#include "influx_renderer/mesh.h"
#include "influx_renderer/texture.h"
#include "influx_renderer/scene.h"
#include "influx_renderer/stats.h"
#include "influx_renderer/shadertoy.h"
#include "influx_renderer/postprocess.h"

// influx::shader
#include "influx_shader.h"

namespace influx::renderer
{
	template <typename _t>
	using result = influx::result<_t, const char*>;

	// shader data
	struct shader_data final
	{
		shader::e_shader_type	m_type;
		shader::reflection		m_reflection;
		vector<byte>			m_bytecode;
		time::point				m_time_loaded;
		uint32					m_num_times_loaded = 0u;

		inline bool is_newer_than(const time::point& timepoint) const
		{
			return m_time_loaded > timepoint;
		}

		INFLUX_RENDER_API
		static shader_data translate(const shader::compile_output& compile_output);
	};

	// 1. initialize the renderer
	enum class e_log { info, warning, error, count };
	typedef void (*log_function)(e_log, const char*);
	struct init_args final
	{
		log_function m_log_func = nullptr;

		e_render_api m_api_type = e_render_api::dx12;

		// if empty, the default source folder is used
		string m_shader_source_folder = "";
	};
	INFLUX_RENDER_API void initialize(const init_args& args);
	INFLUX_RENDER_API bool is_initialized();

	// end. release your resources!
	INFLUX_RENDER_API void cleanup();

	// 1. acquire the frame to render
	INFLUX_RENDER_API void start_frame();

	INFLUX_RENDER_API // - 3D scene rendering
	result<bool> draw_scene(const scene& scene, const target& target);

	INFLUX_RENDER_API // - imgui rendering
	result<bool> draw_imgui(ImDrawData const* draw_data, const target& target);
	INFLUX_RENDER_API 
	result<bool> draw_imgui(const vector<ImDrawData const*>& draws, const vector<target const*>& targets);

	INFLUX_RENDER_API // - sprite rendering
	result<bool> draw_2D(const scene2D& scene, const target& target);

 	INFLUX_RENDER_API // - shadertoy rendering
	result<bool> draw_shadertoy(const scene_shadertoy& scene, const target& target);

	INFLUX_RENDER_API // - postprocess rendering
	result<bool> draw_postprocess(const scene_postprocess& scene, const target& target);

	INFLUX_RENDER_API result<bool> can_draw_postprocess();
	INFLUX_RENDER_API result<bool> can_draw_imgui();
	INFLUX_RENDER_API result<bool> can_draw_scene();
	INFLUX_RENDER_API result<bool> can_draw_2D();
	INFLUX_RENDER_API result<bool> can_draw_debug();

	// targets
	INFLUX_RENDER_API target* create_target(const target_create_args& args);

	// creates / switches to the appropriate target representation of our window backbuffer
	INFLUX_RENDER_API target* get_window_target(const platform::window& window);

	// 3. (optional) copy intermediate data
	INFLUX_RENDER_API void copy_target(const target& source, const target& dest);

	struct clear_args final
	{
		math::vectorf4 m_colour = {};
	};
	INFLUX_RENDER_API void clear_target(const target&, const clear_args&);

	// 3. 
	INFLUX_RENDER_API void end_frame();

	// 4. present to window swapchain
	struct present_args final
	{
		bool m_vsync = false;
	};
	INFLUX_RENDER_API void present_all(const present_args& args);
	INFLUX_RENDER_API void present(const platform::window&, const present_args& args);

	INFLUX_RENDER_API void wait_gpu_finished();

	// loading assets into the renderer
	INFLUX_RENDER_API void load(const string& title, const mesh_data<vertex_data>& data, bool reload = false);
	INFLUX_RENDER_API void load(const string& title, const texture_data& data, bool reload = false);
	INFLUX_RENDER_API void load(const string& title, const cubemap_data& data, bool reload = false);
	INFLUX_RENDER_API void load(const shader::shader_signature& signature, const shader_data& data, bool reload = false);
	INFLUX_RENDER_API void load(const string& title, const material& data, bool reload = false);

	INFLUX_RENDER_API time::point get_time_loaded_shader(const shader::shader_signature& signature);
	INFLUX_RENDER_API time::point get_time_loaded_texture(const string& title);
	INFLUX_RENDER_API time::point get_time_loaded_texturecube(const string& title);
	INFLUX_RENDER_API time::point get_time_loaded_mesh(const string& title);

	INFLUX_RENDER_API bool has_mesh(const string& title);
	INFLUX_RENDER_API bool has_texture(const string& title);
	INFLUX_RENDER_API bool has_texturecube(const string& title);
	INFLUX_RENDER_API bool has_shader(const shader::shader_signature& signature);
	INFLUX_RENDER_API bool has_material(const string& title);

	struct render_settings final
	{
		enum class cullmode { back, front, none };
		cullmode m_cullmode = cullmode::back;
		bool m_wireframe = false;
	};
	INFLUX_RENDER_API void set_settings(const render_settings& settings);
	INFLUX_RENDER_API render_settings get_settings();

	// get ImTextureID from a loaded-in texture
	INFLUX_RENDER_API void* get_imgui_texture_id(const string& title);

	// graphics info
	struct memory_info final
	{
		size_t m_gpu_usage = 0u;
		size_t m_gpu_budget = 0u;
	};
	INFLUX_RENDER_API memory_info get_memory_info();

	struct pipeline_info final
	{
		uint32 m_num_pipelines;
	};
	INFLUX_RENDER_API pipeline_info get_pipeline_info();
}