#pragma once

// influx::core
#include "core/material/material.h"

// influx::renderer
#include "influx_renderer.h"
#include "influx_renderer/renderer_imgui.h"
#include "influx_renderer/multimesh.h"
#include "influx_renderer/resources/resource_manager.h"

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
		result<bool> draw_debug(const scene_debug& scene, const target& target);
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
		static graphics::device& get_device();

		void load(const string& title, const mesh_data& data, bool reload = false);
		void load(const string& title, const texture_data& data, bool reload = false);
		void load(const shader::shader_signature& signature, const shader_data& data, bool reload = false);
		void load(const string& title, const material& data, bool reload = false);

		bool has_mesh(const string& title) const;
		bool has_texture(const string& title) const;
		bool has_shader(const shader::shader_signature& signature) const;
		bool has_material(const string& title) const;

		time::point get_time_loaded_shader(const shader::shader_signature& signature) const;
		time::point get_time_loaded_texture(const string& title) const;
		time::point get_time_loaded_mesh(const string& title) const;

		void set_settings(const render_settings& settings);
		const render_settings& get_settings() const;

		texture* create_texture(const string& title, const texture_desc& args);
		const umap<string, texture*>& get_textures() const;
		texture* find_texture(const string& name);
		texture& get_default_texture(); // "none"

		const umap<string, material> get_materials() const;
		material* get_material(const string& name);
		static material& get_default_material(); // "none"

		void upload_texture_data(texture* target_tex, const texture_data& data);

		vector<string> get_mesh_names() const;
		bool get_mesh_buffers(const string& name, graphics::resource*& out_vertex_buffer, graphics::resource*& out_index_buffer);

		memory_info get_memory_info() const;
		pipeline_info get_pipeline_info() const;
		void* get_imgui_texture_id(const string& title);

		template <typename _tvtx>
		graphics::resource* create_vertexbuffer(const string& title, const vector<_tvtx>& data, bool reload = false);
		graphics::resource* create_indexbuffer(const string& title, const vector<index>& data, bool reload = false);

		vector<vertex_data> get_vertexbuffer_content(const string& title) const;
		vector<index> get_indexbuffer_content(const string& title) const;

		const multimesh& get_multimesh() const;

		static bool allow_bindless();

	private:
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

		// resources
		// mesh data
		multimesh m_multimesh{};
		umap<string, graphics::resource*> m_vertex_buffers;
		umap<string, graphics::resource*> m_index_buffers;
		umap<string, vector<index>> m_index_buffer_contents;
		umap<string, vector<vertex_data>> m_vertex_buffer_contents; // can be any type
		umap<string, material> m_materials;
		resource_manager m_resource_manager;

		// texture data
		umap<string, texture*> m_textures;

		render_settings m_settings;

		void recreate_backbuffer_targets(swapchain& swapchain);
		target* get_current_window_target(swapchain& swapchain);
	};

	template<typename _tvtx>
	inline graphics::resource* renderer_backend::create_vertexbuffer(const string& title, const vector<_tvtx>& data, bool reload)
	{
		using vertex_type = _tvtx;

		const uint64 old_bytesize = m_vertex_buffers.contains(title) ? m_vertex_buffers[title]->get_bytesize() : 0u;
		const uint64 new_bytesize = data.size() * sizeof(vertex_type);
		if (old_bytesize < new_bytesize)
		{
			if (m_vertex_buffers[title])
			{
				// release previous if existing
				mp_device->release(m_vertex_buffers[title]);
			}

			// create vertex buffer on the shared heap (so cpu can write to it)
			graphics::heap_desc heap_desc{};
			heap_desc.m_type = graphics::e_heap_type::shared;

			// set default resource state to read
			graphics::buffer_desc desc{};
			desc.m_init_state = graphics::e_resource_state::gen_read;

			// create resource
			desc.m_bytesize = new_bytesize;
			desc.m_bytestride = sizeof(vertex_type);
			m_vertex_buffers[title] = mp_device->create_resource(desc, heap_desc);
			m_vertex_buffers[title]->set_name("vb_" + title);

			m_vertex_buffer_contents[title].resize(data.size());
		}

		if (old_bytesize < new_bytesize || reload)
		{
			m_vertex_buffers[title]->map([&data, new_bytesize](void* target)
			{
				memcpy(target, data.data(), new_bytesize);
			});

			memcpy(m_vertex_buffer_contents[title].data(), data.data(), new_bytesize);
		}

		return m_vertex_buffers[title];
	}
}
