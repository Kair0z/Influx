#pragma once

// influx::core
#include "core/material/material.h"

// influx::renderer
#include "influx_renderer.h"
#include "influx_renderer/renderer_imgui.h"
#include "influx_renderer/multimesh.h"

// influx::graphics
#include "influx_graphics/resource.h"
#include "influx_graphics/device.h"

#pragma region declarations
// influx::graphics
namespace influx::graphics
{
	class device;
	class queue;
	class swapchain;
	class commandlist;
	class command_allocator;
	class fence;
}

// influx::rendergraph
namespace influx::rendergraph
{
	class rendergraph;
}

// influx::renderer
namespace influx::renderer
{
	class descriptor_manager;
	class shader_manager;
	class upload_manager;
	class imgui_manager;
	class pipeline_manager;
	class scene_renderer;
	class debug_renderer;
	class quad_renderer;
	class shadertoy_renderer;
	class target;
	class resource_manager;
	class multimesh;
}

// influx::platform
namespace influx::platform
{
	class window;
}
#pragma endregion

namespace influx::renderer
{	
	class renderer_backend final : public singleton<renderer_backend>
	{
		struct swapchain;

		// konstants
		constexpr static uint32 k_max_instances = 1024u;
		constexpr static e_buffering k_buffering = e_buffering::tripple;

		template <shader::e_shader_type _t>
		using shader_map = umap<string, shader_data>;

	public:
		static void log(e_log, const char* message);

		void initialize(const init_args& args);
		bool is_initialized() const;
		void wait_gpu_finished() const;
		void cleanup();

		void start_frame();
		void end_frame();

		target* create_target(const target_create_args& args);
		target* get_window_target(const platform::window& window);
		void acquire_swapchain_frame(swapchain& swapchain);

		result<bool> draw_scene(const scene& scene, const target& target);
		result<bool> draw_imgui(ImDrawData const* draw_data, const target& target);
		result<bool> draw_imgui(const vector<ImDrawData const*>& draws, const vector<target const*>& targets);
		result<bool> draw_2D(const scene2D& scene, const target& target);
		result<bool> draw_shadertoy(const scene_shadertoy& scene, const target& target);
		result<bool> draw_postprocess(const scene_postprocess& scene, const target& target);

		result<bool> can_draw_postprocess() const;
		result<bool> can_draw_imgui() const;
		result<bool> can_draw_scene() const;
		result<bool> can_draw_2D() const;
		result<bool> can_draw_debug() const;

		void copy_target(const target& source, const target& dest);
		void clear_target(const target&, const clear_args&);
		void present_all(const present_args& args);
		void present(const platform::window& window, const present_args& args);

		static shader_manager& get_shader_manager();
		static descriptor_manager* get_descriptor_manager();
		static upload_manager* get_upload_manager();
		static pipeline_manager* get_pipeline_manager();
		static resource_manager& get_resource_manager();
		static graphics::queue& get_graphics_queue();
		static graphics::device& get_device();

		void load(const string& title, const mesh_data<vertex_data>& data, bool reload = false);
		void load(const string& title, const texture_data& data, bool reload = false);
		void load(const string& title, const cubemap_data& data, bool reload = false);
		void load(const shader::shader_signature& signature, const shader_data& data, bool reload = false);
		void load(const string& title, const material& data, bool reload = false);

		bool has_mesh(const string& title) const;
		bool has_texture(const string& title) const;
		bool has_texturecube(const string& title) const;
		bool has_shader(const shader::shader_signature& signature) const;
		bool has_material(const string& title) const;

		time::point get_time_loaded_shader(const shader::shader_signature& signature) const;
		time::point get_time_loaded_texture(const string& title) const;
		time::point get_time_loaded_texturecube(const string& title) const;
		time::point get_time_loaded_mesh(const string& title) const;

		void set_settings(const render_settings& settings);
		const render_settings& get_settings() const;

		texture2D* create_texture(const string& title, const texture_desc& args);
		cubemap* create_texturecube(const string& title, const cubemap_desc& args);
		const umap<string, texture2D*>& get_textures() const;
		texture2D* find_texture(const string& name);
		cubemap* find_texturecube(const string& name);
		texture2D& get_default_texture(); // "none"

		const umap<string, material> get_materials() const;
		material* get_material(const string& name);
		static material& get_default_material(); // "none"

		void upload_texture_data(texture2D* target_tex, const texture_data& data);

		vector<string> get_mesh_names() const;
		bool get_mesh_buffers(const string& name, graphics::resource*& out_vertex_buffer, graphics::resource*& out_index_buffer);
		bool get_mesh_buffers(const mesh_id& id, graphics::resource*& out_vertex_buffer, graphics::resource*& out_index_buffer);

		memory_info get_memory_info() const;
		pipeline_info get_pipeline_info() const;
		void* get_imgui_texture_id(const string& title);

		static bool allow_bindless();

	private:
		init_args m_init_args{};
		uint64 m_frame_count = 0u;
		bool m_is_initialized = false;

		// rendergraph
		rendergraph::rendergraph* m_rendergraph = nullptr;

		// graphics engine
		graphics::device* mp_device = nullptr;
		graphics::queue* mp_graphics_queue = nullptr;
		graphics::commandlist* mp_commandlist = nullptr;
		vector<graphics::command_allocator*> mp_allocators = {};

		// copy engine
		graphics::queue* mp_copy_queue = nullptr;
		graphics::command_allocator* mp_copy_allocator = nullptr;
		graphics::commandlist* mp_copy_commandlist = nullptr;
		graphics::fence* mp_fence = nullptr;
		graphics::fence* mp_copyfence = nullptr;

		// swapchains
		struct swapchain final
		{
			graphics::swapchain*	mp_swapchain = nullptr;
			vector<target*>			m_targets{};
			string					m_windowtitle{};
		};
		umap<platform::window const*, swapchain> m_swapchains{};

		// managers
		descriptor_manager* mp_desc_manager = nullptr;
		shader_manager* mp_shader_manager = nullptr;
		upload_manager* mp_upload_manager = nullptr;
		pipeline_manager* mp_pipeline_manager = nullptr;
		imgui_manager* mp_imgui = nullptr;
		scene_renderer* mp_scene_renderer = nullptr;
		debug_renderer* mp_debug_renderer = nullptr;
		quad_renderer* mp_quad_renderer = nullptr;
		shadertoy_renderer* mp_shadertoy_renderer = nullptr;
		resource_manager* m_resource_manager;
		render_settings m_settings;

		void recreate_backbuffer_targets(swapchain& swapchain);
		target* get_current_window_target(swapchain& swapchain);
	};
}
