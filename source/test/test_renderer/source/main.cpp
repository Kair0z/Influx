// influx::platform
#include "influx_platform/window.h"
#include "influx_platform/monitor.h"
// influx::renderer
#include "influx_renderer.h"
#include "influx_renderer/pipeline.h"
// influx::core
#include "core/math/vectortools.h"
#include "core/math/random.h"
#include "core/time.h"
#include "core/basetypes.h"
// influx::import
#include "influx_import.h"
// STL
#include <iostream>

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

	// create windows
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
#if 1
	const bool filepath_set = cv_filepath.is_set();
	const string fbx_filepath = cv_filepath.get_value<string>();
	vector<renderer::mesh_id> mesh_ids{};
	vector<renderer::matrix> mesh_transforms{};
	if (filepath_set && path::exists(fbx_filepath))
	{
		imp::scene_load_args args{};
		args.m_bake_transforms;
		args.m_multithreading;
		args.m_pre_scale;
		auto load_res = imp::load_scene_file(fbx_filepath, args);
		if (load_res.is_success())
		{
			imp::scene_data& scene = load_res.get();
			for (const auto& mesh : scene.get_meshes())
			{
				const string name = "leblanc." + scene.get_name(mesh);
				renderer::mesh_data mesh_data{};
				mesh_data.m_indices = mesh.m_indices;
				mesh_data.m_vertices.resize(mesh.m_positions.size());
				for (uint32 i = 0u; i < mesh.m_positions.size(); ++i)
				{
					mesh_data.m_vertices[i].m_position = mesh.m_positions[i];
					// mesh_data.m_vertices[i].m_colour = mesh.m_colours[i];
					mesh_data.m_vertices[i].m_normal = mesh.m_normals[i];
					mesh_data.m_vertices[i].m_texcoords = mesh.m_uvs[i];
				}
				const renderer::mesh_id id = renderer::load(name, mesh_data, false);
				mesh_ids.push_back(id);
				mesh_transforms.push_back(scene.get_transform(mesh));
			}
		}
	}
#else
	renderer::mesh_id triangle_id = renderer::make_id("triangle");
	{
		using vertex = renderer::vertex_data;
		using mesh = renderer::mesh_data<vertex>;
		mesh msh{};
		renderer::load(triangle_id, msh);
	}
#endif

	// setup render world & view
	renderer::world world{};
	for (uint32 i = 0u; i < mesh_ids.size(); ++i)
	{
		world.add_mesh_instance(mesh_ids[i], mesh_transforms[i]);
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

		// render:
		renderer::start_frame();
		for (uint32 i = 0u; i < num_windows; ++i)
		{
			renderer::target* window_target = renderer::get_or_create_window_target(*windows[i]);
			static const renderer::colour clear_colours[] { {1,0,0}, {0,1,0}, {0,0,1} };
			renderer::clear_args clear{ .m_colour = clear_colours[ i % 3 ] };

			renderer::clear_target(*window_target, clear);
			// renderer::draw_world(wview, *window_target);
			renderer::draw_world_with_pipeline("pipeline.lua", wview);
		}
		renderer::end_frame();
		renderer::present_all(present_args);

		// std::cout << renderer::get_last_rendergraph_dotfile().get_std() << "\n";
	}
	renderer::cleanup();
}