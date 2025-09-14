// influx::core
#include "core/math/vectortools.h"
#include "core/math/random.h"
#include "core/time.h"
#include "core/basetypes.h"
#include "core/math/matrix.h"

// DX12 SDK 1.614.1
extern "C" { __declspec(dllexport) extern const influx::uint32 D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

// STL
#include <iostream>

// influx::platform
#include "influx_platform/window.h"
#include "influx_platform/monitor.h"

// influx::graphics
#include "influx_graphics/device.h"

// influx::import
#include "influx_shader.h"

using namespace influx;

struct constants
{
	math::matrix4x4f mat_vp = math::matrix4x4f::identity();
	math::float3 light_direction{};
	float delta_seconds = 0.0f;
	float seconds = 0.0f;
} g_constants{};

void compile_shaders(graphics::mesh_pipeline_desc& out_desc)
{
	const char* include_folder = "E:/Git/Influx/assets/engine/shaders/include/";
	const char* filepath = "E:/Git/Influx/assets/engine/shaders/source/mesh_shaders.hlsl";

	shader::compile_args compile_args{};
	compile_args.set_target(shader::e_shader_target::_6_6);
	compile_args.m_reflection_enabled = false;
	compile_args.m_include_folder = include_folder;

	auto parsed_shaders = shader::parse_shaders_in_file(filepath).get();

	vector<shader::compile_output> compiled_shaders{};
	for (const auto& pair : parsed_shaders.m_shadermap)
		for (const auto& shader : pair.second)
		{
			compile_args.m_target = shader::e_shader_target::_6_6;
			auto res = shader::compile_shader_in_file(filepath, shader.m_signature, compile_args);
			influx_assert(res.is_success());
			compiled_shaders.push_back(res.get());
		}

	for (const shader::compile_output& shader : compiled_shaders)
	{
		out_desc.m_shaders.set(shader.m_signature.m_type, shader.m_bytecode);
	}
}

struct pipeline final
{
	graphics::rootsignature* m_rootsig = nullptr;
	graphics::mesh_pipeline* m_pipeline = nullptr;
};
void create_pipeline(graphics::device& device, pipeline& out_pipeline)
{
	// empty root signature (no bound resources)
	graphics::rootsignature_desc rootsig_desc{};
	rootsig_desc.add_root_constants(sizeof(constants) / sizeof(float), 0u);
	out_pipeline.m_rootsig = device.create_rootsignature(rootsig_desc);

	// describe the pipeline & compile shaders
	graphics::mesh_pipeline_desc pipeline_desc{};
	compile_shaders(pipeline_desc);

	pipeline_desc.m_prim_type = graphics::e_primitive_topology_type::triangle;
	pipeline_desc.set_sample_desc(1u, (uint32)-1);
	pipeline_desc.m_rasterizer = graphics::rasterizer_desc::default_graphics();
	pipeline_desc.m_depth_stencil = graphics::depth_stencil_desc::default_no_stencil();
	pipeline_desc.m_depth_stencil.m_depth_func = graphics::e_comparison_func::less;
	// pipeline_desc.m_rasterizer.m_front_ccw = !pipeline_desc.m_rasterizer.m_front_ccw;

	pipeline_desc.set_rendertarget_desc(0, true, graphics::e_format::rgba8);
	pipeline_desc.set_blend_desc(0, graphics::blend_desc::default_write_all());

	out_pipeline.m_pipeline = device.create_mesh_pipeline(out_pipeline.m_rootsig, pipeline_desc);
}

#pragma region printer helpers
void print(const char* message)
{
	std::cout << "[] " << message << "\n";
}
template <typename _t>
void print_if_unex(const _t& result)
{
	if (result.is_success() == false) print(result.get_unex());
}

struct result_printer final
{
	template <typename _t>
	void operator<<(const _t& result)
	{
		print_if_unex(result);
	}
};
#pragma endregion

int main()
{
	using namespace influx;

	// create window
	vector<platform::monitor> monitors = platform::monitor::query_monitors();
	platform::window_desc window_desc{};
	window_desc.m_dimensions = { 640u, 480u };
	const math::vectoru2 window_half_size = window_desc.m_dimensions / 2;
	window_desc.m_name = "mesh shading";
	platform::window& window = *platform::window::create(window_desc);

	// create device, a cmdlist, queue & window swapchain
	graphics::device& device = *graphics::device::create(graphics::e_api_type::dx12);
	graphics::commandlist& commandlist = *device.create_graphics_commandlist();
	graphics::queue& queue = *device.create_queue();
	graphics::swapchain& swapchain = *device.create_swapchain(&queue, window);
	
	// we need only 1 single rtv allocated (backbuffer)
	graphics::descriptor_heap& rtv_heap = *device.create_descriptor_heap(graphics::descriptor_heap::create_rtv_heap(1u));
	graphics::descriptor_heap& dsv_heap = *device.create_descriptor_heap(graphics::descriptor_heap::create_dsv_heap(1u));
	graphics::descriptor_handle rtv_handle = rtv_heap.allocate_cpu().get();
	graphics::descriptor_handle dsv_handle = dsv_heap.allocate_cpu().get();

	graphics::resource* depth_target = nullptr;
	{
		graphics::tex2D_desc desc{};
		desc.m_arraysize = 1u;
		desc.m_dimensions = window_desc.m_dimensions;
		desc.m_format = graphics::e_format::d32;
		desc.m_num_mips = 1u;
		desc.m_sample_count = 1u;
		desc.m_bindflags = graphics::e_bind_flags::dsv;
		desc.m_init_state = graphics::e_resource_state::depth_target;
		depth_target = device.create_resource(desc);
		device.create_dsv(dsv_handle, depth_target);
	}
	
	// create mesh shader pipeline
	pipeline pipeline{};
	create_pipeline(device, pipeline);

	// misc variables
	graphics::present_args present_args{};
	present_args.m_vsync = false;
	time::point time_last_tick = time::get_now();
	float delta_seconds = 0.0f;
	float seconds = 0.0f;
	bool is_quit = false;

	// update constants
	const float camera_distance = 50;
	math::float3 camera_pos = { 1, 1, 1 };
	camera_pos = camera_pos.normalized() * camera_distance;
	math::float3 camera_lookat = math::float3::make_zero() - camera_pos;
	camera_lookat.normalize();

	const float nearp = 0.001f;
	const float farp = 2000.0f;
	g_constants.mat_vp = math::matrix4x4f::make_viewprojection_RH(
		camera_pos,					// pos
		camera_lookat,				// forward
		90.0f,						// fov
		window.get_aspect_ratio(),	// ar
		nearp,
		farp
	);

	g_constants.light_direction = { 0.5f, -0.5f, -0.5f };
	g_constants.light_direction.normalize();

	// settings
	uint32 grid_dim = 10;
	uint32 num_cubes = grid_dim * grid_dim * grid_dim;
	uint32 num_cubes_per_group = 10;
	uint32 num_groups = math::ceil<uint32>((float)num_cubes / num_cubes_per_group);

	result_printer res{};
	while (is_quit == false)
	{
		delta_seconds = time::get_ms_since<float>(time_last_tick) * 0.001f;
		time_last_tick = time::get_now();
		seconds += delta_seconds;

		g_constants.seconds = seconds;
		g_constants.delta_seconds = delta_seconds;

		// poll OS
		window.poll_events(is_quit);

		res << commandlist.start(&device);

		// transition backbuffer to render target
		graphics::resource* backbuffer = swapchain.get_current_backbuffer_resource().get();
		res << backbuffer->transition(&commandlist, graphics::e_resource_state::render_target);
		
		// set viewport & rect
		res << commandlist.set_vp_and_rect
		(
			{ 0.0f, 0.0f },			// min
			{ 640.0f , 480.0f }		// max
		);

		// create backbuffer rtv (ideally don't recreate each frame, but on DX12, this is cool-ish)
		device.create_rtv(rtv_handle, backbuffer);
		res << commandlist.set_rtv(rtv_handle, dsv_handle);
		res << commandlist.clear_rtv(rtv_handle, {0,0,0,1});
		res << commandlist.clear_dsv(dsv_handle, 1.0f, 0u);

		// set pipeline & sig, then dispatch the mesh shader
		res << commandlist.set_rootsignature(pipeline.m_rootsig);
		res << commandlist.set_pipeline(pipeline.m_pipeline);
		res << commandlist.set_root_constants(0u, sizeof(constants) / sizeof(float), &g_constants, graphics::e_pipeline_type::mesh);
		res << commandlist.dispatch_mesh(num_groups,1,1);

		// transition backbuffer to present
		res << backbuffer->transition(&commandlist, graphics::e_resource_state::present);

		commandlist.submit(&queue);

		swapchain.present(present_args);
	}

	// cleanup..
	// eh...
}