
#include "core/basetypes.h"

// SDK 1.614.1
extern "C" { __declspec(dllexport) extern const influx::uint32 D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

#include "influx_platform/window.h"
#include "influx_platform/monitor.h"

#include "influx_graphics/device.h"
#include "influx_renderer.h"

#include "core/math/vectortools.h"
#include "core/math/random.h"
#include "core/time.h"

#include "influx_import.h"

using namespace influx;

void load_scene(const string& filepath, imp::scene_load_args& args, renderer::scene& out_scene)
{
	const string filename = str::split(str::split(filepath, "/").back(), ".").front();
	static vector<renderer::camera> cameras{};
	static vector<math::matrix4x4f> mesh_transforms{};
	static vector<math::matrix4x4f> camera_transforms{};
	static vector<uint32> mesh_ids{};

	uint32 chosen_camera_idx = 0u;

	// load the scene with the stuff only once
	static bool once = true;
	if (once)
	{
		// load the fbx
		imp::scene_data loaded_scene{};
		influx_assert(imp::load_scene_file(filepath, loaded_scene, args));

		// get the cameras
		for (uint32 i = 0u; i < loaded_scene.m_cameras.size(); ++i)
		{
			const imp::scene_data::camera& camera = loaded_scene.m_cameras[i];
			camera_transforms.push_back(camera.m_world_transform);
			renderer::camera render_camera{};
			render_camera.m_camera.set_fov(90.0f);// camera.m_camera.get_fov();
			render_camera.m_camera.set_farplane(camera.m_camera.get_farplane());
			render_camera.m_camera.set_nearplane(camera.m_camera.get_nearplane());
			cameras.push_back(render_camera);
		}
		// our own camera
		{
			math::transform3D custom_transform = math::transform3D::identity();
			custom_transform.set_position({ 0,0,500 });
			custom_transform.look_at({});
			camera_transforms.push_back(custom_transform.get_matrix());
			
			renderer::camera custom_camera{};
			custom_camera.m_camera.set_farplane(1000.0f);
			custom_camera.m_camera.set_nearplane(0.001f);
			custom_camera.m_camera.set_fov(110.0f);
			cameras.push_back(custom_camera);
		}
		
		// convert the meshes
		for (uint32 i = 0u; i < loaded_scene.get_num_meshes(); ++i)
		{
			const imp::mesh_data& mesh = loaded_scene.get_mesh(i);

			// convert imp:: to renderer::
			influx::renderer::mesh_data render_data{};
			render_data.m_vertices.reserve(mesh.m_positions.size());
			render_data.m_indices.reserve(mesh.m_indices.size());
			for (uint64 i = 0u; i < mesh.m_positions.size(); ++i)
			{
				renderer::vertex_data data{};
				data.m_position = mesh.m_positions[i];
				// data.m_colour	= main_mesh.m_colours[i];
				data.m_normal = mesh.m_normals[i];
				data.m_texcoords = mesh.m_uvs[i];
				render_data.m_vertices.push_back(data);
			}
			for (uint64 i = 0u; i < mesh.m_indices.size(); ++i)
			{
				render_data.m_indices.push_back(mesh.m_indices[i]);
			}

			const string mesh_name = filename + "_" + to_string(i);
			mesh_ids.push_back(i);
			mesh_transforms.push_back(mesh.m_world_transform);

			// load to renderer
			renderer::load(mesh_name, render_data, false);
		}
		once = false;
	}

	// choose the camera
	out_scene.set_camera(
		cameras[chosen_camera_idx],
		camera_transforms[chosen_camera_idx]);

	// add all meshes
	for (uint32 i = 0u; i < mesh_ids.size(); ++i)
	{
		out_scene.add_mesh(mesh_ids[i], mesh_transforms[i]);
	}
}

void load_shaders()
{
	using namespace influx;

	// shaders
	static const string shaders_folder = "D:/Git/Influx/assets/engine/shaders/";

	// global args
	shader::compile_args args{};
	args.m_include_folder = shaders_folder;
	args.m_signature.m_target = shader::e_shader_target::_6_6;
	args.m_reflection = true;
	args.m_defines = {};
	args.m_compile_debug = INFLUX_DEBUG;
	args.m_pbd = INFLUX_DEBUG;
	args.m_pdb_folder = "D:/Git/Influx/int/shaderdebug/";

	imp::shader_data loaded_shaders[5u]{};

	vector<imp::shader_data> all_shaders{};
	imp::load_shader_file(shaders_folder + "/source/debug_shaders.hlsl", all_shaders, args);

	// vs
	args.m_signature.m_type = shader::e_shader_type::vs;
	args.m_signature.m_entrypoint = "main_vs";
	args.m_signature.m_filename = "debug_shaders";
	args.m_signature.cache_id();
	influx_assert(imp::load_shader_file(shaders_folder + "/source/debug_shaders.hlsl", loaded_shaders[0], args));

	args.m_signature.m_type = shader::e_shader_type::ps;
	args.m_signature.m_filename = "debug_shaders";
	args.m_signature.m_entrypoint = "main_ps";
	args.m_signature.cache_id();
	influx_assert(imp::load_shader_file(shaders_folder + "/source/debug_shaders.hlsl", loaded_shaders[1], args));


	args.m_signature.m_type = shader::e_shader_type::vs;
	args.m_signature.m_entrypoint = "main_vs";
	args.m_signature.m_filename = "basepass";
	args.m_signature.cache_id();
	influx_assert(imp::load_shader_file(shaders_folder + "/source/basepass.hlsl", loaded_shaders[2], args));

	args.m_signature.m_type = shader::e_shader_type::ps;
	args.m_signature.m_filename = "basepass";
	args.m_signature.m_entrypoint = "main_ps";
	args.m_signature.cache_id();
	influx_assert(imp::load_shader_file(shaders_folder + "/source/basepass.hlsl", loaded_shaders[3], args));

	args.m_signature.m_type = shader::e_shader_type::cs;
	args.m_signature.m_filename = "resolvepass";
	args.m_signature.m_entrypoint = "main_cs";
	args.m_signature.cache_id();
	influx_assert(imp::load_shader_file(shaders_folder + "/source/resolvepass.hlsl", loaded_shaders[4], args));

	renderer::shader_data render_shaders[5u]{};
	for (uint32 i = 0u; i < 5u; ++i)
	{
		render_shaders[i].m_bytecode = loaded_shaders[i].m_compile_result.m_bytecode;
		render_shaders[i].m_reflection = loaded_shaders[i].m_compile_result.m_reflection;
		render_shaders[i].m_type = loaded_shaders[i].m_signature.m_type;
		render_shaders[i].m_time_loaded = time::get_now();
	}

	renderer::load(loaded_shaders[0].m_signature, render_shaders[0]);
	renderer::load(loaded_shaders[1].m_signature, render_shaders[1]);
	renderer::load(loaded_shaders[2].m_signature, render_shaders[2]);
	renderer::load(loaded_shaders[3].m_signature, render_shaders[3]);
	renderer::load(loaded_shaders[4].m_signature, render_shaders[4]);
}

void set_quaternion_scene(renderer::scene& scene)
{
	static math::matrix4x4f transform = math::matrix4x4f::identity();
	static bool once = true;
	if (once)
	{
		scene.set_camera(
			renderer::camera{},
			
			math::matrix4x4f::make_transform_RH(
			{ 0, 0, 10 }, // position
			{ 0, 0, -1 }  // forward
		));

		math::boxf box = math::boxf::identity();
		scene.add_gizmo_transform(math::transform3D::identity());
		scene.add_box(box.get_transformed3D(transform), colour::k_red);
		once = false;
	}
}

int main()
{
	using namespace influx;
	using namespace influx::renderer;

	vector<platform::monitor> monitors = platform::monitor::query_monitors();
	platform::window_desc window_desc{};
	window_desc.m_dimensions = { 360, 360 };
	const math::vectoru2 window_half_size = window_desc.m_dimensions / 2;
	window_desc.m_name = "renderer";

	static constexpr uint32 num_windows = 3u;
	platform::window* windows[num_windows] =
	{
		platform::window::create(window_desc.set_name("A")),
		platform::window::create(window_desc.set_name("B")),
		platform::window::create(window_desc.set_name("c")),
	};

	// initialize renderer
	renderer::init_args render_init{};
	render_init.m_api_type = renderer::e_render_api::dx12;
	influx::renderer::initialize(render_init);

	// load shaders
	load_shaders();

	// present
	renderer::present_args present_args{};
	present_args.m_vsync = false;

	// setup the render-scene
	renderer::scene scene_to_draw{};
	imp::scene_load_args scene_load_args{};
	scene_load_args.m_bake_transforms = false;
	scene_load_args.m_pre_scale = 1.0f;
	load_scene("D:/Git/Influx/assets/engine/meshes/box.fbx", scene_load_args, scene_to_draw);
	set_quaternion_scene(scene_to_draw);

	static const math::colour_rgba clear_colours[num_windows]
	{
		colour::k_red,
		colour::k_green,
		colour::k_blue
	};

	time::point time_last_tick = time::get_now();
	float delta_seconds = 0.0f;
	float seconds = 0.0f;
	while (true)
	{
		delta_seconds = time::get_ms_since<float>(time_last_tick) * 0.001;
		time_last_tick = time::get_now();
		seconds += delta_seconds;

		// update:
		const uint32 radius = 200;
		for (uint32 i = 0u; i < num_windows; ++i)
		{
			const platform::monitor& monitor = monitors[2];
			const math::vectoru2 monitor_center = monitor.get_rect().get_mid();
			const float angle = seconds + (i * math::k_PIDouble * 0.33f);
			uint32 x = radius * math::cos(angle);
			uint32 y = radius * math::sin(angle);
			windows[i]->set_position(monitor_center + math::vectoru2{ x,y } - window_half_size);
		}

		// render:
		renderer::target* window_targets[num_windows]
		{
			renderer::get_window_target(*windows[0u]),
			renderer::get_window_target(*windows[1u]),
			renderer::get_window_target(*windows[2u]),
		};

		renderer::start_frame();

		for (uint32 i = 0u; i < num_windows; ++i)
		{
			renderer::clear_args clear{ .m_colour = clear_colours[i] };
			renderer::clear_target(*window_targets[i], clear);
		}
		
		renderer::draw_scene(scene_to_draw, *window_targets[1]);

		renderer::end_frame();

		for (uint32 i = 0u; i < num_windows; ++i)
		{
			renderer::present(*windows[i], present_args);
		}
	}
}