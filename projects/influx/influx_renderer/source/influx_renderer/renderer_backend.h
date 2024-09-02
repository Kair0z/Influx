#pragma once
#include "influx_renderer.h"
#include "influx_renderer/renderer_imgui.h"

#pragma region declarations
// influx::graphics
namespace influx::graphics
{
	class device;
	class command_queue;
	class swapchain;
	class command_list;
	class command_allocator;
	class fence;
}

// influx::renderer
namespace influx::renderer
{
	class descriptor_manager;
	class upload_manager;
	class imgui_manager;
	class pipeline_manager;
	class scene_renderer;
	class target;
}
#pragma endregion

namespace influx::renderer
{
	class renderer_backend final
		: public singleton<renderer_backend>
	{
		// konstants
		constexpr static uint32 k_max_instances = 1024u;
		constexpr static e_buffering k_buffering = e_buffering::tripple;

	public:
		void initialize(const init_args& args);
		bool is_initialized() const;
		void cleanup();

		target* create_target(const target_create_args& args);
		target* get_window_target(const platform::window_handle& window);
		void acquire_swapchain_frame();

		void draw_scene(const scene& scene, const target& target);
		void draw_imgui(ImDrawData* draw_data, const target& target);
		void copy_target(const target& source, const target& dest);
		void present_swapchain(const present_args& args);
		static descriptor_manager* get_descriptor_manager();
		static upload_manager* get_upload_manager();
		static pipeline_manager* get_pipeline_manager();

		void load(const string& title, const mesh_data& data);
		void load(const string& title, const texture_data& data);
		void load(const string& title, const shader_data& data);
		void load(const string& title, const material& data);

		texture* create_texture(const texture_create_args& args);
		const vector<texture*>& get_textures() const;
		texture* get_texture(const string& name) const;

		const umap<string, material> get_materials() const;
		material* get_material(const string& name);

		void upload_texture_data(texture* target_tex, const texture_data& data);

		vector<string> get_mesh_names() const;
		bool get_mesh_buffers(const string& name, graphics::resource*& out_vertex_buffer, graphics::resource*& out_index_buffer);

	private:
		uint64 m_frame_count = 0u;
		bool m_is_initialized = false;

		// graphics engine
		graphics::device* mp_device = nullptr;
		graphics::command_queue* mp_graphics_queue = nullptr;
		graphics::command_list* mp_commandlist = nullptr;
		vector<graphics::command_allocator*> mp_allocators = {};

		// copy engine
		graphics::command_queue* mp_copy_queue = nullptr;
		graphics::command_allocator* mp_copy_allocator = nullptr;
		graphics::command_list* mp_copy_commandlist = nullptr;
		graphics::fence* mp_fence = nullptr;
		graphics::fence* mp_copyfence = nullptr;

		// swapchain
		graphics::swapchain* mp_swapchain = nullptr;
		vector<target*> m_swapchain_targets{};

		// managers:
		descriptor_manager* mp_desc_manager = nullptr;
		upload_manager* mp_upload_manager = nullptr;
		pipeline_manager* mp_pipeline_manager = nullptr;
		imgui_manager* mp_imgui = nullptr;
		scene_renderer* mp_scene_renderer = nullptr;

		// resources
		umap<string, graphics::resource*> m_vertex_buffers;
		umap<string, graphics::resource*> m_index_buffers;
		umap<string, material> m_materials;
		umap<string, shader_data> m_vertex_shaders;
		umap<string, shader_data> m_pixel_shaders;
		vector<texture*> m_textures;
	};
}
