#pragma once

// influx::core
#include "core/material/material.h"

// influx::renderer
#include "influx_renderer.h"
#include "influx_renderer/renderer_imgui.h"
#include "influx_renderer/renderer_backend.h"

// influx::rendergraph

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
}

// influx::platform
namespace influx::platform
{
	class window;
}
#pragma endregion

namespace influx::renderer
{
	template <typename _vt>
	class multi_vertexbuffer_t final
	{
		using vertex_type = _vt;

	public:
		void add_mesh(const string& name, const vector<vertex_type>& vertices);
		void remove_mesh(const string& name);

		void update_gpu_buffer();

	private:
		graphics::resource* m_resource;
		graphics::descriptor_handle m_descriptor;
		map<string, uint32> m_name_to_offset_map;
		map<string, vector<vertex_type>> m_name_to_buffer;
		bool m_is_cpu_gpu_different = false;

		vector<vertex_type> m_vertices{};
	};

	using multi_vertexbuffer = multi_vertexbuffer_t<vertex_data>;

	class renderer_backend final : public singleton<renderer_backend>
	{
		// konstants
		constexpr static uint32 k_max_instances = 1024u;
		constexpr static e_buffering k_buffering = e_buffering::tripple;

	public:
		void initialize(const init_args& args);
		bool is_initialized() const;
		void wait_gpu_finished() const;
		void cleanup();

		void start_frame();
		void end_frame();

		target* create_target(const target_create_args& args);
		target* get_window_target(const platform::window& window);
		void acquire_swapchain_frame();

		void draw_scene(const scene& scene, const target& target);
		void draw_imgui(ImDrawData* draw_data, const target& target);
		void draw_2D(const scene2D& scene, const target& target);
		void draw_debug(const scene_debug& scene, const target& target);
		void draw_shadertoy(const scene_shadertoy& scene, const target& target);
		void copy_target(const target& source, const target& dest);
		void clear_target(const target&, const clear_args&);
		void present_swapchain(const present_args& args);

		static descriptor_manager* get_descriptor_manager();
		static upload_manager* get_upload_manager();
		static pipeline_manager* get_pipeline_manager();

		void load(const string& title, const mesh_data& data, bool reload = false);
		void load(const string& title, const texture_data& data, bool reload = false);
		void load(const string& title, const shader_data& data, bool reload = false);
		void load(const string& title, const material& data, bool reload = false);

		bool has_mesh(const string& title) const;
		bool has_texture(const string& title) const;
		bool has_shader(const string& title) const;
		bool has_material(const string& title) const;

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

		umap<string, shader_data>& get_vertex_shaders();
		umap<string, shader_data>& get_pixel_shaders();

		void* get_imgui_texture_id(const string& title);

		template <typename _tvtx>
		graphics::resource* create_vertexbuffer(const string& title, const vector<_tvtx>& data, bool reload = false);
		graphics::resource* create_indexbuffer(const string& title, const vector<index>& data, bool reload = false);

		template <typename _tvtx>
		vector<_tvtx> get_vertexbuffer_content(const string& title) const;
		vector<index> get_indexbuffer_content(const string& title) const;

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

		// swapchain
		graphics::swapchain* mp_swapchain = nullptr;
		vector<target*> m_swapchain_targets{};

		// managers
		descriptor_manager* mp_desc_manager = nullptr;
		upload_manager* mp_upload_manager = nullptr;
		pipeline_manager* mp_pipeline_manager = nullptr;
		imgui_manager* mp_imgui = nullptr;
		scene_renderer* mp_scene_renderer = nullptr;
		debug_renderer* mp_debug_renderer = nullptr;
		quad_renderer* mp_quad_renderer = nullptr;
		shadertoy_renderer* mp_shadertoy_renderer = nullptr;

		// resources
		multi_vertexbuffer m_multi_vertexbuffer{};
		umap<string, graphics::resource*> m_vertex_buffers;
		umap<string, graphics::resource*> m_index_buffers;
		umap<string, vector<index>> m_index_buffer_contents;
		umap<string, vector<uint8>> m_vertex_buffer_contents; // can be any type
		umap<string, material> m_materials;
		umap<string, shader_data> m_vertex_shaders;
		umap<string, shader_data> m_pixel_shaders;
		umap<string, texture*> m_textures;

		render_settings m_settings;

		void recreate_backbuffer_targets();
		target* get_current_window_target();
	};

	template<typename _tvtx>
	inline graphics::resource* renderer::renderer_backend::create_vertexbuffer(const string& title, const vector<_tvtx>& data, bool reload)
	{
		using vertex_type = _tvtx;

		if (!m_vertex_buffers.contains(title))
		{
			// create index / vertex buffer on the shared heap (so cpu can write to it)
			graphics::heap_desc heap_desc{};
			heap_desc.m_type = graphics::e_heap_type::shared;

			// set default resource state to read
			graphics::buffer_desc desc{};
			desc.m_init_state = graphics::e_resource_state::gen_read;

			// create resource
			desc.m_bytesize = data.size() * sizeof(vertex_type);
			desc.m_bytestride = sizeof(vertex_type);
			m_vertex_buffers[title] = mp_device->create_resource(desc, heap_desc);

			// map data to resource
			m_vertex_buffers[title]->map([&data](void* target)
			{
				memcpy(target, data.data(), data.size() * sizeof(vertex_type));
			});

			// preserve contents
			m_vertex_buffer_contents[title].resize(desc.m_bytesize / sizeof(uint8));
			memcpy(m_vertex_buffer_contents[title].data(), data.data(), desc.m_bytesize);

			m_vertex_buffers[title]->set_name("vb_" + title);

			m_multi_vertexbuffer.add_mesh(title, data);
		}

		return m_vertex_buffers[title];
	}

	template<typename _tvtx>
	inline vector<_tvtx> renderer::renderer_backend::get_vertexbuffer_content(const string& title) const
	{
		if (m_vertex_buffer_contents.contains(title))
		{
			const vector<uint8>& data = m_vertex_buffer_contents.at(title);
			const uint64 bytesize = (data.size() * sizeof(uint8));
			const uint64 num_vertices = bytesize / sizeof(_tvtx);
			vector<_tvtx> result(num_vertices);
			memcpy(result.data(), data.data(), bytesize);
			return result;
		}

		return {};
	}

	template<typename _vt>
	inline void renderer::multi_vertexbuffer_t<_vt>::add_mesh(const string& name, const vector<_vt>& vertices)
	{
		m_name_to_buffer[name] = vertices;
		m_is_cpu_gpu_different = true;
	}

	template<typename _vt>
	inline void renderer::multi_vertexbuffer_t<_vt>::remove_mesh(const string& name)
	{
		m_is_cpu_gpu_different = true;
	}

	template<typename _vt>
	inline void renderer::multi_vertexbuffer_t<_vt>::update_gpu_buffer()
	{
		if (m_is_cpu_gpu_different && !m_name_to_buffer.empty())
		{
			for (const auto& pair : m_name_to_buffer)
			{
				const string& name = pair.first;
				const auto& vertices = pair.second;


			}


			uint64 total_bytesize = 0u;
			vector<gpu_vertex_data> gpu_data{};

			// gather a mega gpu_vertexdata vector
			for (const string& name : m_meshnames)
			{
				const vector<vertex_data> vertexbuffer_content = backend.get_vertexbuffer_content<vertex_data>(name);
				if (vertexbuffer_content.size() > 0u)
				{
					const uint64 old_size = gpu_data.size();
					const uint64 num_vertices = vertexbuffer_content.size();
					const uint64 bytesize = num_vertices * sizeof(vertex_data);
					gpu_data.resize(old_size + num_vertices);

					// copy the individual vertexbuffer content into our gpu data mega-vector
					memcpy(&gpu_data[old_size], vertexbuffer_content.data(), bytesize);

					// keep the base offset
					m_meshname_to_offset[name] = old_size;

					total_bytesize += bytesize;
				}
			}
		}

		m_is_cpu_gpu_different = false;
	}
}
