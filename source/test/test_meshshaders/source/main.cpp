
#include "core/basetypes.h"

// SDK 1.614.1
extern "C" { __declspec(dllexport) extern const influx::uint32 D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

// influx::platform
#include "influx_platform/window.h"
#include "influx_platform/monitor.h"

// influx::graphics
#include "influx_graphics/device.h"

// influx::core
#include "core/math/vectortools.h"
#include "core/math/random.h"
#include "core/time.h"

// influx::import
#include "influx_import.h"

using namespace influx;

void compile_shaders(graphics::mesh_pipeline_desc& out_desc)
{
	vector<imp::shader_data> loaded_shaders{};

	shader::compile_args compile_args{};
	compile_args.set_target(shader::e_shader_target::_6_6);
	compile_args.m_reflection = false;

	compile_args.m_include_folder = "E:/Git/Influx/assets/engine/shaders/include/";
	bool load_success = imp::load_shader_file("E:/Git/Influx/assets/engine/shaders/source/mesh_shaders.hlsl", loaded_shaders, compile_args);
	influx_assert(load_success);

	for (const imp::shader_data& shader : loaded_shaders)
	{
		out_desc.m_shaders.set(shader.m_type, shader.m_compile_result.m_bytecode);
	}
}

struct pipeline final
{
	graphics::rootsignature* m_rootsig = nullptr;
	graphics::mesh_pipeline* m_pipeline = nullptr;
};
void create_pipeline(graphics::device& device, pipeline& out_pipeline)
{
	graphics::rootsignature_desc rootsig_desc{};
	graphics::mesh_pipeline_desc pipeline_desc{};
	compile_shaders(pipeline_desc);
	out_pipeline.m_rootsig = device.create_rootsignature(rootsig_desc);

	pipeline_desc.m_prim_type = graphics::e_primitive_topology_type::triangle;
	pipeline_desc.m_rasterizer.m_cullmode = graphics::e_cull_mode::nocull;
	pipeline_desc.m_rasterizer.m_fillmode = graphics::e_fill_mode::solid;

	pipeline_desc.m_rasterizer.m_forced_samplecount = 0u;
	pipeline_desc.m_sample_mask = (uint32)-1;
	pipeline_desc.m_sample_count = 1u;
	pipeline_desc.m_rasterizer.m_front_ccw = false;
	pipeline_desc.m_rasterizer.m_depth_clip_enable = false;
	pipeline_desc.m_rasterizer.m_multisample = false;
	pipeline_desc.m_rasterizer.m_antialiased_line = false;
	pipeline_desc.m_rasterizer.m_conservative = false;
	pipeline_desc.m_rasterizer.m_depth_bias = 0;
	pipeline_desc.m_rasterizer.m_depth_bias_clamp = 0.0f;
	pipeline_desc.m_rasterizer.m_slope_depth_bias = 0.0f;

	pipeline_desc.m_depth_stencil.m_depth_enable = true;
	pipeline_desc.m_depth_stencil.m_stencil_enable = false;
	pipeline_desc.m_depth_stencil.m_depth_func = graphics::e_comparison_func::less;
	pipeline_desc.m_depth_stencil.m_format = graphics::e_format::d32;

	pipeline_desc.m_rtvs[0].m_enabled = true;
	pipeline_desc.m_rtvs[0].m_format = graphics::e_format::rgba8;
	pipeline_desc.m_blends[0].m_write_mask = 15u;

	out_pipeline.m_pipeline = device.create_mesh_pipeline(out_pipeline.m_rootsig, pipeline_desc);
}

int main()
{
	using namespace influx;

	// create window
	vector<platform::monitor> monitors = platform::monitor::query_monitors();
	platform::window_desc window_desc{};
	window_desc.m_dimensions = { 640u, 480u };
	const math::vectoru2 window_half_size = window_desc.m_dimensions / 2;
	window_desc.m_name = "raytracing";
	platform::window& window = *platform::window::create(window_desc);

	// create graphics stuff
	graphics::device& device = *graphics::device::create(graphics::e_api_type::dx12);
	graphics::commandlist& commandlist = *device.create_graphics_commandlist();
	graphics::queue& queue = *device.create_queue();
	graphics::swapchain& swapchain = *device.create_swapchain(&queue, window);
	graphics::descriptor_heap& rtv_heap = *device.create_descriptor_heap(
		graphics::descriptor_heap::create_args::rtv_heap(1));

	graphics::descriptor_handle rtv_handle = rtv_heap.allocate_cpu();
	
	// create pipeline
	pipeline pipeline{};
	create_pipeline(device, pipeline);

	graphics::present_args present_args{};
	present_args.m_vsync = false;
	time::point time_last_tick = time::get_now();
	float delta_seconds = 0.0f;
	float seconds = 0.0f;
	bool is_quit = false;
	while (is_quit == false)
	{
		delta_seconds = time::get_ms_since<float>(time_last_tick) * 0.001f;
		time_last_tick = time::get_now();
		seconds += delta_seconds;

		window.poll_events(is_quit);

		// render
		swapchain.acquire_backbuffer();
		commandlist.start(&device);

		graphics::resource* backbuffer = swapchain.get_current_backbuffer_resource();
		backbuffer->transition(
			&commandlist, graphics::e_resource_state::render_target);

		commandlist.set(graphics::viewport
		{
			.m_left = 0.0f,
			.m_top = 0.0f,
			.m_width = 640.0f,
			.m_height = 480.0f
		});
		commandlist.set(graphics::rect
		{
			.m_left = 0u,
			.m_top = 0u,
			.m_right = 640u,
			.m_bottom = 480u
		});

		device.create_rtv(rtv_handle, backbuffer);
		commandlist.clear_rtv(rtv_handle, {1,0,0,1});
		commandlist.set_rtv(rtv_handle, nullptr);

		commandlist.set(pipeline.m_rootsig);
		commandlist.set(pipeline.m_pipeline);
		commandlist.dispatch_mesh(1,1,1);
		
		backbuffer->transition(
			&commandlist, graphics::e_resource_state::present);

		commandlist.submit(&queue);

		swapchain.present(present_args);
	}

	// cleanup..
}