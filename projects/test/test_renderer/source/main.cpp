
#include "core/basetypes.h"

// SDK 1.614.1
extern "C" { __declspec(dllexport) extern const influx::uint32 D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

#include "influx_platform/window.h"
#include "influx_graphics/device.h"
#include "influx_renderer.h"

#include "core/math/vectortools.h"
#include "core/math/random.h"

#include "influx_import.h"

const influx::renderer::mesh_data& get_mesh_data(
	influx::math::matrix4x4f& out_transform,
	influx::math::matrix4x4f& out_cam_transform)
{
	using namespace influx;

	// load the fbx
	imp::scene_data loaded_scene{};

	imp::scene_load_args args{};
	args.m_bake_transforms = false;
	args.m_pre_scale = 1;
	influx_assert(imp::load_scene_file("D:/Git/Influx/assets/engine/meshes/box.fbx", loaded_scene, args));

	out_cam_transform = loaded_scene.m_cameras[0u].m_world_transform;

	const imp::mesh_data& main_mesh = loaded_scene.get_main_mesh();
	out_transform = main_mesh.m_world_transform;
	static influx::renderer::mesh_data result{};
	{
		result.m_vertices.reserve(main_mesh.m_positions.size());
		result.m_indices.reserve(main_mesh.m_indices.size());

		for (uint64 i = 0u; i < main_mesh.m_positions.size(); ++i)
		{
			renderer::vertex_data data{};
			data.m_position = main_mesh.m_positions[i];
			// data.m_colour	= main_mesh.m_colours[i];
			data.m_normal	= main_mesh.m_normals[i];
			data.m_texcoords = main_mesh.m_uvs[i];
			result.m_vertices.push_back(data);
		}

		for (uint64 i = 0u; i < main_mesh.m_indices.size(); ++i)
		{
			result.m_indices.push_back(main_mesh.m_indices[i]);
		}
	}
	return result;
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

	imp::shader_data loaded_shaders[3u]{};

	// vs
	args.m_signature.m_type = shader::e_shader_type::vs;
	args.m_signature.m_entrypoint = "main_vs";
	args.m_signature.m_filename = "basepass";
	args.m_signature.cache_id();
	influx_assert(imp::load_shader_file(shaders_folder + "/source/basepass.hlsl", loaded_shaders[0], args));

	args.m_signature.m_type = shader::e_shader_type::ps;
	args.m_signature.m_filename = "basepass";
	args.m_signature.m_entrypoint = "main_ps";
	args.m_signature.cache_id();
	influx_assert(imp::load_shader_file(shaders_folder + "/source/basepass.hlsl", loaded_shaders[1], args));

	args.m_signature.m_type = shader::e_shader_type::cs;
	args.m_signature.m_filename = "resolvepass";
	args.m_signature.m_entrypoint = "main_cs";
	args.m_signature.cache_id();
	influx_assert(imp::load_shader_file(shaders_folder + "/source/resolvepass.hlsl", loaded_shaders[2], args));

	renderer::shader_data render_shaders[3u]{};
	for (uint32 i = 0u; i < 3u; ++i)
	{
		render_shaders[i].m_bytecode = loaded_shaders[i].m_compile_result.m_bytecode;
		render_shaders[i].m_reflection = loaded_shaders[i].m_compile_result.m_reflection;
		render_shaders[i].m_type = loaded_shaders[i].m_signature.m_type;
		render_shaders[i].m_time_loaded = time::get_now();
	}

	renderer::load(loaded_shaders[0].m_signature, render_shaders[0]);
	renderer::load(loaded_shaders[1].m_signature, render_shaders[1]);
	renderer::load(loaded_shaders[2].m_signature, render_shaders[2]);
}

int main()
{
	using namespace influx;
	using namespace influx::renderer;

	platform::window_desc window_desc{};
	window_desc.m_dimensions = { 640u, 480u };
	window_desc.m_name = "renderer";

	platform::window* window = platform::window::create(window_desc);

	// initialize renderer
	renderer::init_args render_init{};
	render_init.m_api_type = renderer::e_render_api::dx12;
	influx::renderer::initialize(render_init);

	// present
	renderer::present_args present_args{};
	present_args.m_vsync = false;

	// load assets
	math::matrix4x4f mesh_transform{};
	math::matrix4x4f cam_transform{};
	const auto& mesh_data = get_mesh_data(mesh_transform, cam_transform);
	renderer::load("my_mesh", mesh_data);
	load_shaders();

	// setup camera
	renderer::scene scene_to_draw{};
	scene_to_draw.m_camera.m_fov = 90.0f;
	scene_to_draw.m_camera.m_near_plane = 0.01f;
	scene_to_draw.m_camera.m_far_plane = 1000.0f;
	scene_to_draw.m_camera.m_transform.set_matrix(cam_transform);
	//scene_to_draw.m_camera.m_transform.set_position({ 0,0,10 });
	//scene_to_draw.m_camera.m_transform.update_matrix();
	scene_to_draw.m_camera.m_transform.look_at({});
	scene_to_draw.m_camera.m_transform.update_matrix();
	
#if 0
	// spawn some meshes in a circle
	for (const auto& point : math::get_points_in_circle(5.0f, 32u))
	{
		static const math::vectorf3 offset = { 0,0,0 };
		scene_to_draw.add_mesh("my_mesh", math::matrix4x4f::make_transform_RH(offset + point, point.normalized()));
		scene_to_draw.m_meshes.back().m_per_instance_colour = random::get_random_unit_vectorf3();
	}
#endif

	scene_to_draw.add_mesh("my_mesh", mesh_transform);

	while (true)
	{
		// attach a window target
		renderer::target* window_target = renderer::get_window_target(*window);

		renderer::start_frame();

		renderer::clear_args clear{};
		clear.m_colour = colour::k_black;
		renderer::clear_target(*window_target, clear);

		renderer::draw_scene(scene_to_draw, *window_target);

		renderer::end_frame();
	}
}