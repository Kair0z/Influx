
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

void compile_shaders(graphics::raytracing_pipeline_desc& out_desc)
{
	vector<imp::shader_data> loaded_shaders{};

	shader::compile_args compile_args{};
	compile_args.set_target(shader::e_shader_target::_6_6);

	compile_args.m_include_folder = "D:/Git/Influx/assets/engine/shaders/include/";
	bool load_success = imp::load_shader_file("D:/Git/Influx/assets/engine/shaders/source/raytracing.hlsl", loaded_shaders, compile_args);
	influx_assert(load_success);

	for (const imp::shader_data& shader : loaded_shaders)
	{
		out_desc.m_shaders.set(shader.m_type, shader.m_compile_result.m_bytecode);
	}
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
	
	// create pipeline
	graphics::rootsignature_desc rootsig_desc{};
	graphics::raytracing_pipeline_desc ray_pipeline_desc{};
	compile_shaders(ray_pipeline_desc);
	graphics::rootsignature& ray_rootsignature = *device.create_rootsignature(rootsig_desc);
	graphics::raytracing_pipeline& ray_pipeline = *device.create_raytracing_pipeline(&ray_rootsignature, ray_pipeline_desc);

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
		commandlist.start(&device);
		
		commandlist.submit(&queue);

		swapchain.present(present_args);
	}

	// cleanup..
}