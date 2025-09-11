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
	enum class e_shadersource_directory : uint8
	{
		base,
		include,
		source
	};

	class renderer_backend final : public singleton<renderer_backend>
	{
		struct swapchain;

		// konstants
		constexpr static e_buffering k_buffering = e_buffering::tripple;

		init_args	m_init_args = {};
		uint64		m_frame_count = 0u;
		bool		m_is_initialized = false;
		string		m_shadersource_directory = "";

		target*						m_finaltarget = nullptr;
		umap<debug_name, target*>	m_targets = {};
		rendergraph::rendergraph*	m_rendergraph = nullptr;
		graphics::device*			mp_device = nullptr;
		graphics::queue*			mp_graphics_queue = nullptr;
		graphics::commandlist*		mp_commandlist = nullptr;
		graphics::fence*			m_gpu_finished_fence = nullptr;

		struct swapchain final
		{
			target*					m_finaltarget_proxy = nullptr;
			graphics::swapchain*	mp_swapchain = nullptr;
			string					m_windowtitle{};
		};
		umap<platform::window const*, swapchain> m_swapchains{};

		descriptor_manager*		mp_desc_manager = nullptr;
		upload_manager*			mp_upload_manager = nullptr;
		pipeline_manager*		mp_pipeline_manager = nullptr;
		imgui_manager*			mp_imgui = nullptr;
		scene_renderer*			mp_scene_renderer = nullptr;
		quad_renderer*			mp_quad_renderer = nullptr;
		shadertoy_renderer*		mp_shadertoy_renderer = nullptr;
		resource_manager*		m_resource_manager = nullptr;
		render_settings			m_settings = {};

	public:
		renderer_backend();
		static void log(e_log, const char* message);

		void initialize(const init_args& args);
		bool is_initialized() const;
		void wait_gpu_finished() const;
		void cleanup();

		void start_frame();
		void end_frame();

		result<target*> create_target(const target_create_args& args);
		result<> destroy_target(target*& target);

		target* get_or_create_window_target(const platform::window& window);
		void acquire_swapchain_frame(swapchain& swapchain);

		result<> draw_scene(const scene& scene, const target& target);
		result<> draw_imgui(ImDrawData const* draw_data, const target& target);
		result<> draw_imgui(const vector<ImDrawData const*>& draws, const vector<target const*>& targets);
		result<> draw_2D(const scene2D& scene, const target& target);
		result<> draw_postprocess(const scene_postprocess& scene, const target& target);

		bool can_draw_postprocess() const;
		bool can_draw_imgui() const;
		bool can_draw_scene() const;
		bool can_draw_2D() const;
		bool can_draw_debug() const;

		void copy_target(const target& source, const target& dest);
		void clear_target(const target&, const clear_args&);
		void present_all(const present_args& args);
		void present(const platform::window& window, const present_args& args);

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

		mesh_id get_mesh_id(e_mesh) const;

		time::point get_time_loaded_shader(const shader::shader_signature& signature) const;
		time::point get_time_loaded_texture(const string& title) const;
		time::point get_time_loaded_texturecube(const string& title) const;
		time::point get_time_loaded_mesh(const string& title) const;

		// rendergraph stuff
		result<> import_to_graph(const target& target);
		string get_last_executed_rendergraph_dump();

		void set_settings(const render_settings& settings);
		const render_settings& get_settings() const;

		texture2D* find_texture(const string& name);
		cubemap* find_texturecube(const string& name);
		texture2D& get_default_texture(); // "none"

		void upload_texture_data(texture2D* target_tex, const texture_data& data);

		vector<string> get_mesh_names() const;
		bool get_mesh_buffers(const mesh_id& id, graphics::resource*& out_vertex_buffer, graphics::resource*& out_index_buffer);

		memory_info get_memory_info() const;
		pipeline_info get_pipeline_info() const;
		rendergraph_info get_rendergraph_info() const;

		static bool allow_bindless();

		string get_shadersource_directory(e_shadersource_directory _enum = e_shadersource_directory::base) const;

	private:
		void recreate_backbuffer_finaltarget(swapchain& swapchain);
	};
}
