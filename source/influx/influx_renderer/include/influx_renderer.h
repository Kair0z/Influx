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
#include "core/commandline.h"
#include "core/plugin.h"

// influx::platform
#include "influx_platform/window.h"

// influx::renderer
#include "influx_renderer/common.h"
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
		- these can be changed at runtime at anytime
	*/
	struct render_settings final
	{
		enum class cullmode { back, front, none };
		cullmode	m_cullmode = cullmode::back;
		bool		m_wireframe = false;
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
		int m_argc; char** m_argv;

		log_function	m_log_func = nullptr;
		e_render_api	m_api_type = e_render_api::dx12;
		render_settings m_init_settings = {};
	};

	INFLUX_RENDER_API void initialize(const init_args& args);
	INFLUX_RENDER_API bool is_initialized();

	/* 
		releases all resources
		past calling this, it's unsafe to call any of the rest of the API! 
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

	/* draw ImDrawData contents onto a given render target */
	INFLUX_RENDER_API result<> draw_imgui(ImDrawData const* draw_data, const target& target);
	INFLUX_RENDER_API result<> draw_imgui(const vector<ImDrawData const*>& draws, const vector<target const*>& targets);

	/* draw a post-processing stack onto a given render target*/
	INFLUX_RENDER_API result<> draw_postprocess(const scene_postprocess& scene, const target& target);

	/* draw a world onto a given render target */
	INFLUX_RENDER_API result<> draw_world(const worldview& view, const target& target);

	/* draw using a parsed pipeline at filepath, using world as input data */
	INFLUX_RENDER_API result<> draw_world_with_pipeline(const char* pipeline_filepath, const worldview& world);

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
	INFLUX_RENDER_API void wait_until_gpu_idle();

	/* 
		loading resources into the backend renderer:
		- meshes (index + vertex buffer)
		- textures
		- cubemaps
		- shaders
		- materials
	*/
	INFLUX_RENDER_API void load(const mesh_id& id, const mesh_data<vertex_data>& data, bool reload = false);
	INFLUX_RENDER_API void load(const tex_id& id, const texture2D_data& data, bool reload = false);
	INFLUX_RENDER_API void load(const cubemap_id& id, const cubemap_data& data, bool reload = false);
	INFLUX_RENDER_API void load(const shader_id& id, const shader_data& data, bool reload = false);
	INFLUX_RENDER_API void load(const mat_id& id, const material& data, bool reload = false);

	/* get the time at which a resource with a given signature was last loaded in (useful for hot-reloading) */
	INFLUX_RENDER_API time::point get_time_loaded_shader(const shader_id& id);
	INFLUX_RENDER_API time::point get_time_loaded_texture(const tex_id& id);
	INFLUX_RENDER_API time::point get_time_loaded_cubemap(const cubemap& id);
	INFLUX_RENDER_API time::point get_time_loaded_mesh(const mesh_id& id);

	/* returns whether a resource with a given signature is loaded in the backend */
	INFLUX_RENDER_API bool has_mesh(const mesh_id& id);
	INFLUX_RENDER_API bool has_texture(const tex_id& id);
	INFLUX_RENDER_API bool has_cubemap(const cubemap_id& id);
	INFLUX_RENDER_API bool has_shader(const shader_id& signature);
	INFLUX_RENDER_API bool has_material(const mat_id& id);

	INFLUX_RENDER_API vector<tex_id> get_loaded_meshes();
	INFLUX_RENDER_API vector<tex_id> get_loaded_textures();
	INFLUX_RENDER_API vector<tex_id> get_loaded_cubemaps();
	INFLUX_RENDER_API vector<tex_id> get_loaded_shaders();
	INFLUX_RENDER_API vector<tex_id> get_loaded_materials();

	INFLUX_RENDER_API string get_resource_name(object_id id);
	INFLUX_RENDER_API string get_mesh_name(const mesh_id& id);

	INFLUX_RENDER_API result<cptr<texture2D>> get_texture2D(const tex_id& id);
	INFLUX_RENDER_API result<cptr<texture3D>> get_texture3D(const tex_id& id);

	/* returns the signature of internal meshes represented by e_mesh */
	INFLUX_RENDER_API mesh_id get_mesh_id(e_mesh internal_mesh);

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

	/* rendergraph info */
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
	INFLUX_RENDER_API string get_last_rendergraph_dotfile();
	INFLUX_RENDER_API string get_last_rendergraph_dump();

	// inline helpers
	inline shader_id load(const shader::shader_signature& signature, const shader_data& data, bool reload = false) {
		const shader_id id = make_shader_id(signature);
		load(id, data, reload);
		return id;
	}
	inline mesh_id load(const string& unique_name, const mesh_data<vertex_data>& data, bool reload = false) {
		const mesh_id id = make_mesh_id(unique_name);
		load(id, data, reload);
		return id;
	}
	inline tex_id load(const string& unique_name, const texture2D_data& data, bool reload = false) {
		const tex_id id = make_tex_id(unique_name);
		load(id, data, reload);
		return id;
	}
	inline cubemap_id load(const string& unique_name, const cubemap_data& data, bool reload = false) {
		const cubemap_id id = make_cubemap_id(unique_name);
		load(id, data, reload);
		return id;
	}

	inline bool has_mesh(const string& unique_name) {
		return has_mesh(make_mesh_id(unique_name));
	}
	inline bool has_texture(const string& unique_name) {
		return has_texture(make_tex_id(unique_name));
	}
	inline bool has_cubemap(const string& unique_name) {
		return has_cubemap(make_cubemap_id(unique_name));
	}
	inline result<cptr<texture2D>> get_texture2D(const string& unique_name) {
		return get_texture2D(make_tex_id(unique_name));
	}
	inline result<cptr<texture3D>> get_texture3D(const string& unique_name) {
		return get_texture3D(make_tex_id(unique_name));
	}


	// define the plugin interface
	class INFLUX_RENDER_API plugin final : plugin_interface
	{
	public:

	};
}