#include "engine_pch.h"

// influx::platform
#include "influx_platform/platform.h"
#include "influx_platform/window.h"

// influx::async
#include "influx_async.h"

// influx::input
#include "influx_input.h"

// influx::renderer
#include "influx_renderer/scene.h"

// influx::engine
#include "content/content_manager.h"
#include "rendering/render_manager.h"
#include "editor/editor_manager.h"
#include "game/game_manager.h"
#include "world/world.h"

// influx::core
#include "core/math/vectortools.h"
#include "core/math/random.h"

namespace influx::engine
{
	void engine::run(run_type type)
	{
		m_runtype = type;
		initialize();

		// TEMP:
		// little scene with camera controls and central mesh
		{
			const uint32 num_swords = 50u;
			const math::circlef3D circle = math::circlef3D({}, { 0,1,0 }, 2.0f);
			const auto points = math::get_points_in_circle(circle, num_swords);
			for (uint32 i = 0u; i < num_swords; ++i)
			{
				auto entity = m_world->create_entity();
				transform_component& ent_transform = m_world->create_component<transform_component>(entity);
				ent_transform.set_position(points[i]);
				//ent_transform.set_scale(0.1f);

				mesh_component& ent_mesh = m_world->create_component<mesh_component>(entity);
				ent_mesh.set_mesh_name("transistor");
				ent_mesh.set_use_normalized_scale(true); // scales to bounding sphere
				ent_mesh.set_invert_normals(false);

				material_component& ent_mat = m_world->create_component<material_component>(entity);

				if (i % 2 == 0)
				{
					ent_mat.set_texture(e_texture_semantic::basecolor, "T_Sword_Opaque_BC");
				}
				else
				{
					ent_mat.set_texture(e_texture_semantic::basecolor, "lego");
				}
				
				ent_mat.set_texture(e_texture_semantic::normals, "");
				ent_mat.set_texture(e_texture_semantic::roughness, "");
			}
			
			static float distance = 5.0f;
			auto camera = m_world->create_entity();
			transform_component& cam_transform = m_world->create_component<transform_component>(camera);
			m_world->create_component<camera_component>(camera).set_fov(90.0f);

			static math::float2 angular_position = {};
			static math::float2 mouse_position_previous = {};
			input_component& cam_input = m_world->create_component<input_component>(camera);
			cam_input.m_on_mouse_move = [this, &cam_transform](const input::mouse_position& pos)
			{
				const float ar = get_window()->get_rect(platform::window::e_space::client).get_aspect_ratio();
				math::float2 delta_mouse = (pos.m_client - mouse_position_previous);
				delta_mouse.y *= ar; // normalize mousemove
				mouse_position_previous = pos.m_client;

				const float seconds = m_time.get_time_seconds();
				const float delta_seconds = m_time.get_delta_seconds();
				angular_position += delta_mouse * delta_seconds * 0.5f;

				cam_transform.set_position_x(distance * math::sinf(angular_position.y) * math::cosf(angular_position.x));
				cam_transform.set_position_y(distance * math::cosf(angular_position.y));
				cam_transform.set_position_z(distance * math::sinf(angular_position.y) * math::sinf(angular_position.x));
				cam_transform.look_at({});
			};
		}

		while (!m_is_quit_requested)
		{
			influx_scope("frame");

			m_time.tick();
			m_fps = 1.0f / m_time.get_delta_seconds();

			// platform window tick
			{
				influx_scope("poll_window");
				poll_platform_events();
				if (m_is_quit_requested) break;
			}

			// main update
			{
				influx_scope("update");
				m_world->update();
			}

			// stream available assets from content into the renderer
			{
				influx_scope("render_upload");
				m_renderman->load_render_assets(m_contentman);
			}

			// record imgui
			if (type == run_type::editor)
			{
				influx_scope("record_imgui");
				m_renderman->record_imgui_frame([this](ImGuiContext& ctx)
				{
					m_editorman->update_imgui(ctx);
				});
			}
			
			// build a render-scene
			renderer::scene scene{};
			{
				influx_scope("build_renderscene");
				scene.m_seconds = m_time.get_time_seconds();
				scene.m_delta_seconds = m_time.get_delta_seconds();
				renderer::scene2D scene2D{};
				m_world->build_renderscene(scene, scene2D, m_renderman->get_debug_render());
			}
			{
				influx_scope("render");
				m_renderman->render(scene);
			}
		}

#if INFLUX_DEBUG
		influx::log_scopedata();
#endif

		cleanup();
		m_is_quit = true;
	}

	void engine::initialize()
	{
		random::seed_random(0u);

		m_t_init = time::get_now();

		// setup engine config
		m_config.m_file_influx_root = get_engine_directory(engine_directory::root);
		m_config.m_file_influx_assets = get_engine_directory(engine_directory::assets);
		m_config.m_file_influx_staged = get_engine_directory(engine_directory::staged);

		// initialize job system:
		async::init_args async_args{};
		async_args.m_num_workers = 1u;
		async::initialize(async_args);

		// initialize input and run an input thread
		influx::input::init();
		m_inputthread = thread([this]()
		{
			run_input();
		});

		// initialize content
		m_contentman = new content_manager(this);
		m_contentthread = thread([this]()
		{
			// init content
			m_contentman->load_engine_assets(this);

			while (!m_is_quit_requested)
			{

			}
		});

		// initialize editor
		if (m_runtype == run_type::editor) 
			m_editorman = new editor_manager(nullptr);
		
		m_gameman = new game_manager();

		// initialize render
		string render_name = (m_runtype == run_type::editor) ? "influx_editor" : "influx_game";
		const math::vectoru2 window_dimensions = { 1280u, 720u };
		initialize_renderer(render_name, window_dimensions);

		// init world
		m_world = new world();
	}

	void engine::run_input()
	{
		while (!m_is_quit_requested)
		{
			input::service();
		}
	}

	void engine::cleanup()
	{
		async::shutdown();

		if (m_editorman)
		{
			delete m_editorman;
			m_editorman = nullptr;
		}

		if (m_renderman)
		{
			delete m_renderman;
			m_renderman = nullptr;
		}

		if (m_contentman)
		{
			delete m_contentman;
			m_contentman = nullptr;
		}

		if (m_inputthread.joinable())
			m_inputthread.join();

		if (m_world)
		{
			delete m_world;
			m_world = nullptr;
		}
	}

	void engine::initialize_renderer(const string& window_name, const math::vectoru2& size)
	{
		platform::window_desc window_desc{};
		window_desc
			.set_dimensions(size)
			.set_name(window_name);

		m_window = platform::window::create(window_desc);
		if (m_window == nullptr)
		{
			logonce(e_log_category::warning, "influx_engine::engine: window::create() failed!");
		}

		m_window->set_event_callback([this](const platform::window_event& ev)
		{
			on_window_event(ev);
		});

		m_renderman = new render_manager(this);
	}

	void engine::poll_platform_events()
	{
		m_window->poll_events(m_is_quit_requested);
		m_is_quit_requested |= m_window->has_quit_request();
	}

	void engine::on_window_event(const platform::window_event& ev)
	{
		input::push_window_event(ev);
	}

	result<cptr<platform::window>> engine::get_window() const
	{
		return m_window;
	}

	result<cptr<content_manager>> engine::get_content() const
	{
		return m_contentman;
	}

	result<cptr<render_manager>> engine::get_renderer() const
	{
		return m_renderman;
	}

	result<cptr<world>> engine::get_world() const
	{
		return m_world;
	}

	result<ptr<content_manager>> engine::get_content()
	{
		return m_contentman;
	}

	result<ptr<game_manager>> engine::get_game()
	{
		return m_gameman;
	}

	ptr<world> engine::get_world_ptr()
	{
		return m_world;
	}

	const frame_time& engine::get_time() const
	{
		return m_time;
	}

	float engine::get_fps() const
	{
		return m_fps;
	}
	
	bool engine::is_quit() const
	{
		return m_is_quit;
	}

	bool engine::is_editor() const
	{
		return m_runtype == run_type::editor;
	}

	bool engine::is_game() const
	{
		return m_runtype == run_type::game;
	}

	result<cptr<world>> get_world()
	{
		return get_engine()->get_world();
	}
}

#include "influx_engine.h"
namespace influx::engine
{
	void run_editor()
	{
		get_engine()->run(engine::run_type::editor);
	}

	void run_game()
	{
		get_engine()->run(engine::run_type::game);
	}
}