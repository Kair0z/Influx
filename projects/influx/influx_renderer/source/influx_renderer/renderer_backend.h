#pragma once
#include "influx_renderer.h"
#include "influx_renderer/rendersystem.h"

#pragma region graphics declarations
namespace influx::graphics
{
	class device;
	class command_queue;
	class swapchain;
	class command_list;
	class command_allocator;
	class fence;
	class pipeline;
	class rootsignature;
}
#pragma endregion

namespace influx::renderer
{
	class descriptor_manager;
	class target;

	class renderer_backend final
		: public singleton<renderer_backend>
	{
		// konstants
		constexpr static e_buffering k_buffering = e_buffering::tripple;

	public:
		void initialize(const init_args& args);
		bool is_initialized() const;
		void cleanup();

		target* create_target(const target_create_args& args);
		target* get_window_target(const platform::window_handle& window);

		void acquire_swapchain_frame();

		void draw_scene(const scene& scene, const target& target);

		void copy_target(const target& source, const target& dest);

		void present_swapchain(const present_args& args);

		static descriptor_manager* get_descriptor_manager();

		void load(const string& title, const mesh_data& data);
		void load(const string& title, const texture_data& data);
		void load(const string& title, const material_data& data);
		void load(const string& title, const shader_data& data);

	private:
		void draw_meshes(const scene& scene, const target& target);
		bool create_pipeline_if_possible();

	private:
		graphics::device* mp_device = nullptr;
		graphics::command_queue* mp_graphics_queue = nullptr;
		graphics::swapchain* mp_swapchain = nullptr;
		graphics::command_list* mp_commandlist = nullptr;
		vector<graphics::command_allocator*> mp_allocators = {};

		graphics::command_queue* mp_copy_queue = nullptr;
		graphics::command_allocator* mp_copy_allocator = nullptr;
		graphics::command_list* mp_copy_commandlist = nullptr;

		graphics::rootsignature* mp_rootsig = nullptr;
		graphics::pipeline* mp_pipeline = nullptr;

		graphics::fence* mp_fence = nullptr;

		vector<target*> m_swapchain_targets{};
		descriptor_manager* mp_desc_manager = nullptr;

		vector<render_args*> mp_rendersystems{};

		uint64 m_frame_count = 0u;
		bool m_is_initialized = false;

		// resources
		map<string, graphics::resource*> m_vertex_buffers;
		map<string, graphics::resource*> m_index_buffers;
		map<string, shader_data> m_vertex_shaders;
		map<string, shader_data> m_pixel_shaders;
		vector<texture*> m_textures;

		void create_render_systems();

		template <class _sys_t>
		_sys_t* get_render_system() const
		{
			auto found = std::find_if(mp_rendersystems.cbegin(), mp_rendersystems.cend(), [](const rendersystem* sys)
			{
				return typeid(*sys) == typeid(_sys_t);
			});

			if (found != mp_rendersystems.cend())
			{
				return static_cast<_sys_t*>(found);
			}
			else
			{
				return nullptr;
			}
		}

		template <class _sys_t, class ..._args>
		_sys_t* create_render_system(_args&& ...args)
		{
			// create new system
			return new _sys_t(args...);
		}
	};
}
