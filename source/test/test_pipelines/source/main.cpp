// influx::platform
#include "influx_platform/window.h"
#include "influx_platform/monitor.h"
// influx::renderer
#include "influx_renderer.h"
// influx::core
#include "core/math/vectortools.h"
#include "core/math/random.h"
#include "core/time.h"
#include "core/basetypes.h"

// SDK 1.614.1
extern "C" { __declspec(dllexport) extern const influx::uint32 D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

static const char* k_invalid_path = "";

// required parameters:
influx::cvar cv_filepath("cv_fbxscene", k_invalid_path, "required: filepath of the model to render");

int main(int argc, char* argv[])
{
	using namespace influx;
	using namespace influx::renderer;
	cvar::parse_runargs(argc, argv);

	// platform setup:
	// - allocate windows
	vector<platform::monitor> monitors = platform::monitor::query_monitors();
	static constexpr uint32 num_windows = 1u;
	platform::window* windows[num_windows] = {};
	platform::window_desc window_desc{};
	window_desc.m_dimensions = { 512u, 512u };
	const math::vectoru2 window_half_size = window_desc.m_dimensions / 2;
	{
		window_desc.m_name = "renderer";
		for (uint32 i = 0u; i < num_windows; ++i)
			windows[i] = platform::window::create(window_desc.set_name(to_string(i)));
	}

	// renderer init:
	{
		renderer::init_args render_init{};
		render_init.m_api_type = renderer::e_render_api::dx12;
		// render_init.m_shader_source_folder = "E:/Git/Influx/assets/engine/shaders/";
		influx::renderer::initialize(render_init);
	}

	// load a triangle mesh into the renderer:
	static constexpr uint32 k_num_triangles = 1024u;
	static const auto k_triangle_id = renderer::make_id("triangle");
	{
		using vertex = renderer::vertex_data;
		using mesh = renderer::mesh_data<vertex>;
		mesh msh{};
		renderer::load(k_triangle_id, msh);
	}

	// setup render world & view
	renderer::world world{};
	for (uint32 i = 0u; i < k_num_triangles; ++i)
	{
		world.add_mesh_instance(k_triangle_id, math::matrix4x4f::identity());
	}
	world.add_light(renderer::light::make_point({ 1,0,0,1 }, 1.0f), renderer::matrix::identity());
	renderer::worldview wview{};

	renderer::camera camera{};
	camera.set_aspect_ratio(1.0f);
	camera.set_farplane(1000.0f);
	camera.set_nearplane(0.001f);
	camera.set_fov(90.0f);
	camera.set_is_orthographic(false);
	wview.m_matrices.update(renderer::matrix::identity(), camera);
	wview.m_world = &world;
	
	// setup the render-scene
	renderer::present_args present_args{}; present_args.m_vsync = false;
	time::point time_last_tick = time::get_now();
	float delta_seconds = 0.0f;
	float seconds = 0.0f;
	bool is_quit = false;
	while (!is_quit)
	{
		// tick:
		delta_seconds = time::get_ms_since<float>(time_last_tick) * 0.001f;
		time_last_tick = time::get_now();
		seconds += delta_seconds;

		// update:
#if 0
		const float radius = 200;
		for (uint32 i = 0u; i < num_windows; ++i)
		{
			const platform::monitor& monitor = monitors[0];
			const math::vectoru2 monitor_center = monitor.get_rect().get_mid();
			const float angle = seconds + (i * math::k_PIDouble / num_windows);
			uint32 x = (uint32)(radius * math::cos(angle));
			uint32 y = (uint32)(radius * math::sin(angle));
			windows[i]->set_position(monitor_center + math::vectoru2{ x,y } - window_half_size);
			windows[i]->poll_events(is_quit);
		}
#endif

		// render:
		renderer::start_frame();
		for (uint32 i = 0u; i < num_windows; ++i)
		{
			renderer::target* window_target = renderer::get_or_create_window_target(*windows[i]);
			static const renderer::colour clear_colours[] { {1,0,0}, {0,1,0}, {0,0,1} };
			renderer::clear_args clear{ .m_colour = clear_colours[ i % 3 ] };

			renderer::clear_target(*window_target, clear);
			renderer::draw_world(wview, *window_target);
		}
		renderer::end_frame();
		renderer::present_all(present_args);

		// std::cout << renderer::get_last_rendergraph_dotfile().get_std() << "\n";
	}
	renderer::cleanup();
}