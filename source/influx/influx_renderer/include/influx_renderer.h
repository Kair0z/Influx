#pragma once

/*
	influx renderer
	is a opaque renderer that just puts things on screen.
	it's not a DIW RHI wrapper, it renders things its own way.

	If you're looking for a lower-level interface to make your own renderer,
	take a peek at influx_graphics
*/

#define INFLUX_RENDER_BINDLESS 1

#pragma region includes
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
#include "influx_renderer/mesh.h"
#include "influx_renderer/texture.h"
#include "influx_renderer/scene.h"
#include "influx_renderer/stats.h"
#include "influx_renderer/postprocess.h"
#include "influx_renderer/shader.h"
#pragma endregion

namespace influx::renderer
{
	/*
		global render settings
		- these can be changed at runtime anytime
	*/
	struct render_settings final
	{
		enum class cullmode { back, front, none };
		cullmode m_cullmode = cullmode::back;
		bool m_wireframe = false;
	};
	INFLUX_RENDER_API void set_settings(const render_settings& settings);
	INFLUX_RENDER_API render_settings get_settings();

	/* 
		initialize the renderer backend 
		- optional logger callback
		- internal graphics API (dx12 / vulkan)
		- optional resource folder to look for internal shaders
	*/
	enum class e_log { info, warning, error, count };
	typedef void (*log_function)(e_log, const char*);

	struct init_args final
	{
		log_function m_log_func = nullptr;

		e_render_api m_api_type = e_render_api::dx12;

		render_settings m_init_settings = {};

		// if empty, the default source folder is used
		string m_shader_source_folder = "";
	};

	INFLUX_RENDER_API void initialize(const init_args& args);
	INFLUX_RENDER_API bool is_initialized();

	/* 
		cleanup your resources!
		past this point it's unsafe to call any of the rest of the API! 
	*/
	INFLUX_RENDER_API void cleanup();

	/* 
		starts a single frame of rendering 
		- resets the internal render graph
		- acquires a commandlist for recording (may stall if work is still ongoing)
	*/
	INFLUX_RENDER_API void start_frame();

	/*
		submits all rendering work recorded since last begin_frame
		- build & executes the internal render graph
		- submit the commandlists onto the GPU to kick off work
		- stalls the current thread until rendering work is finished
	*/
	INFLUX_RENDER_API void end_frame();

	/* draw a 3D scene onto a given render target */
	INFLUX_RENDER_API result<> draw_scene(const scene& scene, const target& target);

	/* draw ImDrawData contents onto a given render target */
	INFLUX_RENDER_API result<> draw_imgui(ImDrawData const* draw_data, const target& target);
	INFLUX_RENDER_API result<> draw_imgui(const vector<ImDrawData const*>& draws, const vector<target const*>& targets);

	/* draw a screen-space scene onto a given render target (sprite rendering)*/
	INFLUX_RENDER_API result<> draw_2D(const scene2D& scene, const target& target);

	/* draw a post-processing stack onto a given render target*/
	INFLUX_RENDER_API result<> draw_postprocess(const scene_postprocess& scene, const target& target);

	/* query whether the internal shaders & resources required for render-operations are available */
	INFLUX_RENDER_API bool can_draw_postprocess();
	INFLUX_RENDER_API bool can_draw_imgui();
	INFLUX_RENDER_API bool can_draw_scene();
	INFLUX_RENDER_API bool can_draw_2D();
	INFLUX_RENDER_API bool can_draw_debug();

	/* create a render target */
	INFLUX_RENDER_API target* create_target(const target_create_args& args);

	/* creates or gets the existing target representation of a given window (builds the swapchain) */
	INFLUX_RENDER_API target* get_or_create_window_target(const platform::window& window);

	/* copy the contents of a target to another */
	INFLUX_RENDER_API void copy_target(const target& source, const target& dest);

	/* clear the contents of a given render target */
	struct clear_args final
	{
		math::vectorf4 m_colour = {};
	};
	INFLUX_RENDER_API void clear_target(const target&, const clear_args&);

	/* presents a swapchain tied to a given platform window */
	struct present_args final
	{
		bool m_vsync = false;
	};
	INFLUX_RENDER_API void present_all(const present_args& args);

	/* ensure you called get_window_target() at least once on the passed window! 
		to ensure there's a swapchain available to present! */
	INFLUX_RENDER_API void present(const platform::window&, const present_args& args);

	/* stalls the calling thread until all GPU work is finished */
	INFLUX_RENDER_API void wait_gpu_finished();

	/* 
		loading resources into the backend renderer:
		- meshes (index + vertex buffer)
		- textures
		- cubemaps
		- shaders
		- materials
	*/
	INFLUX_RENDER_API void load(const string& title, const mesh_data<vertex_data>& data, bool reload = false);
	INFLUX_RENDER_API void load(const string& title, const texture_data& data, bool reload = false);
	INFLUX_RENDER_API void load(const string& title, const cubemap_data& data, bool reload = false);
	INFLUX_RENDER_API void load(const shader::shader_signature& signature, const shader_data& data, bool reload = false);
	INFLUX_RENDER_API void load(const string& title, const material& data, bool reload = false);

	/* get the time at which a resource with a given signature was last loaded in (useful for hot-reloading) */
	INFLUX_RENDER_API time::point get_time_loaded_shader(const shader::shader_signature& signature);
	INFLUX_RENDER_API time::point get_time_loaded_texture(const string& title);
	INFLUX_RENDER_API time::point get_time_loaded_texturecube(const string& title);
	INFLUX_RENDER_API time::point get_time_loaded_mesh(const string& title);

	/* returns whether a resource with a given signature is loaded in the backend */
	INFLUX_RENDER_API bool has_mesh(const string& title);
	INFLUX_RENDER_API bool has_texture(const string& title);
	INFLUX_RENDER_API bool has_texturecube(const string& title);
	INFLUX_RENDER_API bool has_shader(const shader::shader_signature& signature);
	INFLUX_RENDER_API bool has_material(const string& title);

	/* returns the signature of internal meshes represented by e_mesh */
	INFLUX_RENDER_API mesh_id get_mesh_id(e_mesh);

	/* */
	INFLUX_RENDER_API string get_last_rendergraph_dump();

	/* graphics info */
	struct memory_info final
	{
		size_t m_gpu_usage = 0u;
		size_t m_gpu_budget = 0u;
	};
	INFLUX_RENDER_API memory_info get_memory_info();

	/* pipeline_info */
	struct pipeline_info final
	{
		uint32 m_num_pipelines;
	};
	INFLUX_RENDER_API pipeline_info get_pipeline_info();

	struct rendergraph_info final
	{
		struct pass final
		{
			string m_name;
		};
		using layer = vector<pass>;
		vector<layer> m_layers{};

		struct buffer final
		{
			string m_name;
		};
		struct texture final
		{
			string m_name;
		};
		vector<buffer> m_buffers{};
		vector<texture> m_textures{};
	};
	INFLUX_RENDER_API rendergraph_info get_rendergraph_info();
}