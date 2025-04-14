// influx::core
#include "core/math/vectortools.h"
#include "core/math/random.h"
#include "core/time.h"
#include "core/basetypes.h"

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

void compile_shaders(graphics::mesh_pipeline_desc& out_desc)
{
	const char* include_folder = "D:/Git/Influx/assets/engine/shaders/include/";
	const char* filepath = "D:/Git/Influx/assets/engine/shaders/source/mesh_shaders.hlsl";

	auto parsed_shaders = shader::parse_shaders_in_file(filepath).get();
	vector<shader::compile_output> compiled_shaders{};
	for (const auto& shader : parsed_shaders)
	{
		shader::compile_args compile_args = shader.m_compile_args;
		compile_args.set_target(shader::e_shader_target::_6_6);
		compile_args.m_reflection = false;
		compile_args.m_include_folder = include_folder;

		auto res = shader::compile_shader_in_file(filepath, compile_args);
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
	out_pipeline.m_rootsig = device.create_rootsignature(rootsig_desc);

	// describe the pipeline & compile shaders
	graphics::mesh_pipeline_desc pipeline_desc{};
	compile_shaders(pipeline_desc);

	pipeline_desc.m_prim_type = graphics::e_primitive_topology_type::triangle;
	pipeline_desc.set_sample_desc(1u, (uint32)-1);
	pipeline_desc.m_rasterizer = graphics::rasterizer_desc::default_graphics();
	pipeline_desc.m_depth_stencil = graphics::depth_stencil_desc::default_no_stencil();

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
	graphics::descriptor_heap& rtv_heap = *device.create_descriptor_heap(
		graphics::descriptor_heap::create_rtv_heap(1u));
	graphics::descriptor_handle rtv_handle = rtv_heap.allocate_cpu();
	
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

	result_printer res{};
	while (is_quit == false)
	{
		delta_seconds = time::get_ms_since<float>(time_last_tick) * 0.001f;
		time_last_tick = time::get_now();
		seconds += delta_seconds;

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
			{ 320.0f , 480.0f }		// max
		);

		// create backbuffer rtv (ideally don't recreate each frame, but on DX12, this is cool-ish)
		device.create_rtv(rtv_handle, backbuffer);
		res << commandlist.set_rtv(rtv_handle, nullptr);
		res << commandlist.clear_rtv(rtv_handle, {1,0,0,1});

		// set pipeline & sig, then dispatch the mesh shader
		res << commandlist.set(pipeline.m_rootsig);
		res << commandlist.set(pipeline.m_pipeline);
		res << commandlist.dispatch_mesh(1,1,1);

		// transition backbuffer to present
		res << backbuffer->transition(&commandlist, graphics::e_resource_state::present);

		commandlist.submit(&queue);

		swapchain.present(present_args);
	}

	// cleanup..
	// eh...
}