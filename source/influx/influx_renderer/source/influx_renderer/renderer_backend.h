#pragma once

// influx::core
#include "core/material/material.h"

// influx::renderer
#include "influx_renderer.h"

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
	class world_renderer;
	class debug_renderer;
	class quad_renderer;
	class target;
	class resource_manager;
	class submit_manager;
	class job_manager;
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
		constexpr static e_buffering k_buffering = e_buffering::tripple;

		init_args	m_init_args = {};
		bool		m_is_initialized = false;
		string		m_shadersource_directory = "";

		umap<debug_name, target*>	m_targets = {};
		rendergraph::rendergraph*	m_rendergraph = nullptr;
		graphics::device*			mp_device = nullptr;

		struct swapchain final
		{
			target*					m_finaltarget_proxy = nullptr;
			graphics::swapchain*	mp_swapchain = nullptr;
			string					m_windowtitle{};
		};
		umap<platform::window const*, swapchain> m_swapchains{};

		umap<mesh_id, string> m_mesh_names;

		descriptor_manager*		mp_desc_manager		= nullptr;
		upload_manager*			mp_upload_manager	= nullptr;
		pipeline_manager*		mp_pipeline_manager = nullptr;
		imgui_manager*			mp_imgui			= nullptr;
		world_renderer*			mp_scene_renderer	= nullptr;
		quad_renderer*			mp_quad_renderer	= nullptr;
		resource_manager*		m_resource_manager	= nullptr;
		submit_manager*			m_submit_manager	= nullptr;
		job_manager*			m_job_manager		= nullptr;
		render_settings			m_settings			= {};

	public:
		renderer_backend();
		static void log(e_log, const char* message);

		void initialize(const init_args& args);
		bool is_initialized() const;
		void wait_until_gpu_idle() const;
		void cleanup();

		void start_frame();
		void end_frame();
		uint64 query_gpu_frame();
		uint64 get_cpu_frame() const;

		result<target*> create_target(const target_create_args& args);
		result<> destroy_target(target*& target);

		target* get_or_create_window_target(const platform::window& window);
		void acquire_swapchain_frame(swapchain& swapchain);

		result<> draw_imgui(ImDrawData const* draw_data, const target& target);
		result<> draw_imgui(const vector<ImDrawData const*>& draws, const vector<target const*>& targets);
		result<> draw_postprocess(const scene_postprocess& scene, const target& target);
		result<> draw_world(const worldview& view, const target& target);

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
		static graphics::queue& get_copy_queue();
		static graphics::device& get_device();
		static job_manager& get_jobs();

		static submit_manager& get_submit_manager();

		void load(const mesh_id& id, const mesh_data<vertex_data>& data, bool reload = false);
		void load(const tex_id& id, const texture2D_data& data, bool reload = false);
		void load(const cubemap_id& id, const cubemap_data& data, bool reload = false);
		void load(const shader::shader_signature& signature, const shader_data& data, bool reload = false);
		void load(const mat_id& id, const material& data, bool reload = false);

		bool has_mesh(const mesh_id& id) const;
		bool has_texture(const tex_id& id) const;
		bool has_cubemap(const cubemap_id& id) const;
		bool has_shader(const shader::shader_signature& signature) const;
		bool has_material(const mat_id& id) const;

		const texture2D& get_texture2D(const tex_id& id) const;
		const texture3D& get_texture3D(const tex_id& id) const;

		string get_mesh_name(const mesh_id id) const;
		time::point get_time_loaded_shader(const shader::shader_signature& signature) const;
		time::point get_time_loaded_texture(const tex_id& id) const;
		time::point get_time_loaded_cubemap(const cubemap_id& id) const;
		time::point get_time_loaded_mesh(const mesh_id& id) const;

		// rendergraph stuff
		result<> import_to_graph(const target& target);
		string get_last_executed_rendergraph_dump();

		void set_settings(const render_settings& settings);
		const render_settings& get_settings() const;

		texture2D& get_default_texture(); // "none"

		void upload_texture_data(texture2D* target_tex, const texture2D_data& data);

		vector<string> get_mesh_names() const;
		bool get_mesh_buffers(const mesh_id& id, graphics::resource*& out_vertex_buffer, graphics::resource*& out_index_buffer);

		memory_info get_memory_info() const;
		pipeline_info get_pipeline_info() const;
		rendergraph_info get_rendergraph_info() const;
		string get_last_rendergraph_dotfile() const;
		static bool allow_bindless();

		string get_shadersource_directory(e_shadersource_directory _enum = e_shadersource_directory::base) const;

	private:
		void recreate_backbuffer_finaltarget(swapchain& swapchain);
	};
}
