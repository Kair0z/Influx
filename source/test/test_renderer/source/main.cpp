
#include "core/basetypes.h"

// SDK 1.614.1
extern "C" { __declspec(dllexport) extern const influx::uint32 D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

// influx::platform
#include "influx_platform/window.h"
#include "influx_platform/monitor.h"

// influx::renderer
#include "influx_renderer.h"

// influx::core
#include "core/math/vectortools.h"
#include "core/math/random.h"
#include "core/time.h"

// influx::import
#include "influx_import.h"

using namespace influx;

void load_scene(const string& filepath, imp::scene_load_args& args, renderer::scene& out_scene)
{
	const string filename = str::split(str::split(filepath, "/").back(), ".").front();
	static vector<camera> cameras{};
	static vector<math::matrix4x4f> mesh_transforms{};
	static vector<math::matrix4x4f> camera_transforms{};
	static vector<string> mesh_ids{};

	uint32 chosen_camera_idx = 0u;

	// load the scene with the stuff only once
	static bool once = true;
	if (once)
	{
		// load the fbx
		imp::scene_data loaded_scene = imp::load_scene_file(filepath, args).get();

#if 0
		// get the cameras
		for (uint32 i = 0u; i < loaded_scene.m_cameras.size(); ++i)
		{
			const imp::scene_data::camera& camera = loaded_scene.m_cameras[i];
			camera_transforms.push_back(loaded_scene.get_transform(camera));
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
#endif

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
			const renderer::mesh_id mesh_id = renderer::make_id(mesh_name);
			mesh_ids.push_back(mesh_name);
			mesh_transforms.push_back(loaded_scene.get_transform(mesh));

			// load to renderer
			renderer::load(mesh_id, render_data, false);
		}
		once = false;
	}

	// choose the camera
	out_scene.set_camera(
		cameras[chosen_camera_idx],
		camera_transforms[chosen_camera_idx]);

#if 0
	// add all meshes
	for (uint32 i = 0u; i < mesh_ids.size(); ++i)
	{
		out_scene.add_mesh(mesh_ids[i], mesh_transforms[i]);
	}
#endif
}

void set_quaternion_scene(renderer::scene& scene)
{
	static math::matrix4x4f transform = math::matrix4x4f::identity();
	static bool once = true;
	if (once)
	{
#if 0
		scene.set_camera(
			renderer::camera{},
			
			math::matrix4x4f::make_transform_RH(
			{ 0, 0, 10 }, // position
			{ 0, 0, -1 }  // forward
		));
#endif

		math::boxf box = math::boxf::identity();
		scene.add_gizmo_transform(math::transform3D::identity());
		scene.add_line_box(box.get_transformed3D(transform), colour::k_red);
		once = false;
	}
}

#if 0
renderer::scene make_the_scene()
{
	renderer::scene result{};
	if (true)
	{
#if 0
		// setup camera
		math::transform3D camera_transform = math::transform3D::identity();
		camera_transform.set_position({ 0,0,10.0f });
		camera_transform.look_at({});
		const math::float3 fwd = camera_transform.get_forward();
		renderer::camera camera{};
		camera.m_camera.set_aspect_ratio(1.0f);
		camera.m_camera.set_farplane(1000.0f);
		camera.m_camera.set_nearplane(0.001f);
		camera.m_camera.set_fov(90.0f);
		camera.m_camera.set_is_orthographic(false);
		camera = 0u;
		result.set_camera(camera);
#endif
		result.set_camera_transform(camera_transform.get_matrix());

		static constexpr float room_size = 20.0f;
		static constexpr float plane_scale = 100.0f;
		static const math::float3 side_axis = math::float3::make_forward();
		static const math::float3 back_axis = math::float3::make_right();
		static const math::float4 white_colour = math::float4{ 1,1,1,1 };
		static const math::float4 red_colour = math::float4{ 1,0,0,1 };
		static const math::float4 green_colour = math::float4{ 0,1,0,1 };

		// setup scene
#pragma region geometry
		static constexpr uint32 num_boxes = 2u;
		math::transform3D box_transforms[num_boxes]
		{
			math::transform3D(math::float3{-0.2, 0.0f, 0.0f}, math::rotation::identity(), math::float3{1,1,1}),
			math::transform3D(math::float3{0.2, 0.0f, 0.0f}, math::rotation::identity(), math::float3{1,1,1})
		};
		math::float4 box_colours[num_boxes]
		{
			green_colour,
			red_colour
		};
		static constexpr uint32 num_planes = 5u;
		math::transform3D plane_transforms[num_planes]
		{
			math::transform3D(math::float3{-room_size, 0.0f, 0.0f}, math::rotation::make_angleaxis(side_axis, 90.0f), math::float3{1,1,1} *plane_scale),
			math::transform3D(math::float3{0.0f, room_size, 0.0f}, math::rotation::identity(), math::float3{1,1,1}	*plane_scale),
			math::transform3D(math::float3{room_size, 0.0f, 0.0f}, math::rotation::make_angleaxis(side_axis, -90.0f), math::float3{1,1,1}	*plane_scale),
			math::transform3D(math::float3{0.0f, -room_size, 0.0f}, math::rotation::identity(), math::float3{1,1,1}	*plane_scale),
			math::transform3D(math::float3{0.0f, 0.0f, -room_size}, math::rotation::make_angleaxis(back_axis, 90.0f), math::float3{1,1,1} *plane_scale)
		};
		math::float4 plane_colours[num_planes]
		{
			red_colour,
			white_colour,
			green_colour,
			white_colour,
			white_colour
		};
		static constexpr uint32 num_spheres = 1u;
		math::transform3D sphere_transforms[num_spheres]
		{
			math::transform3D::identity()
		};

		for (uint32 i = 0u; i < num_boxes; ++i)
		{
			auto& mesh = result.add_mesh(renderer::e_mesh::box, box_transforms[i].get_matrix());
			mesh.m_per_instance_colour = box_colours[i];
		}
		for (uint32 i = 0u; i < num_planes; ++i)
		{
			auto& mesh = result.add_mesh(renderer::e_mesh::plane, plane_transforms[i].get_matrix());
			mesh.m_per_instance_colour = plane_colours[i];
		}
		for (uint32 i = 0u; i < num_spheres; ++i)
		{
			auto& sphere = result.add_mesh(renderer::e_mesh::sphere, sphere_transforms[i].get_matrix());
			sphere.m_per_instance_colour = red_colour;
		}
#pragma endregion
#pragma region lights
		static constexpr uint32 num_pointlights = 1u;
		math::float3 plight_positions[num_pointlights]
		{
			math::float3{0,0,0}
		};
		for (uint32 i = 0u; i < num_pointlights; ++i)
		{
			const auto light_trans = math::transform3D(plight_positions[i]);
			auto& light = result.add_light(light::make_point({ 1,1,1,1 }, 1.0f), light_trans.get_matrix());
		}
#pragma endregion
	}
	else
	{
		imp::scene_load_args scene_load_args{};
		scene_load_args.m_bake_transforms = false;
		scene_load_args.m_pre_scale = 1.0f;
		load_scene("D:/Git/Influx/assets/engine/meshes/box.fbx", scene_load_args, result);
	}
	return result;
}
#endif

int main()
{
	using namespace influx;
	using namespace influx::renderer;

	// platform setup:
	// - allocate windows
	vector<platform::monitor> monitors = platform::monitor::query_monitors();
	static constexpr uint32 num_windows = 8u;
	platform::window* windows[num_windows] = {};
	platform::window_desc window_desc{};
	window_desc.m_dimensions = { 128u, 128u };
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
	renderer::mesh_id triangle_id = renderer::make_id("triangle");
	{
		using vertex = renderer::vertex_data;
		using mesh = renderer::mesh_data<vertex>;
		mesh msh{};
		renderer::load(triangle_id, msh);
	}
	
	// setup render world & view
	renderer::world world{};
	math::matrix4x4f transform{};
	world.add_mesh_instance(triangle_id, transform);
	world.add_mesh_instance(triangle_id, transform);
	world.add_light(renderer::light::make_point({ 1,0,0,1 }, 1.0f), transform);
	renderer::worldview wview{};
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
			renderer::draw_world(wview, *window_target);
		}
		renderer::end_frame();
		renderer::present_all(present_args);
	}
	renderer::cleanup();
}