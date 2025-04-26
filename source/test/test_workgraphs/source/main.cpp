// influx::core
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

struct pipeline
{
	graphics::rootsignature* m_rootsignature;
	graphics::graph_pipeline* m_pipeline;
};
pipeline create_pipeline(graphics::device& device)
{
	pipeline result{};

	// compile shaders
	const string shader_filepath = "E:/Git/Influx/assets/engine/shaders/source/workgraph.hlsl";
	auto res = shader::compile_shader_library(shader_filepath, {});
	influx_assert(res.is_success());

	// create rootsig
	{
		graphics::rootsignature_desc desc{};
		result.m_rootsignature = device.create_rootsignature(desc);
	}
	// create pipeline
	{
		graphics::graph_pipeline_desc desc{};
		result.m_pipeline = device.create_workgraph_pipeline(result.m_rootsignature, desc);
	}

	return result;
}

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

	graphics::feature_info features = device.get_feature_info();
	influx_assert(has_flag(features.m_supported_flags, graphics::e_feature_flags::workgraphs));

	// pipeline
	pipeline pipeline = create_pipeline(device);

	// misc variables
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

		// poll OS
		window.poll_events(is_quit);

		commandlist.start(&device);

		// transition backbuffer to render target
		graphics::resource* backbuffer = swapchain.get_current_backbuffer_resource().get();
		backbuffer->transition(&commandlist, graphics::e_resource_state::render_target);
		
		// set viewport & rect
		commandlist.set_vp_and_rect
		(
			{ 0.0f, 0.0f },			// min
			{ 640.0f , 480.0f }		// max
		);

		commandlist.dispatch_workgraph(pipeline.m_pipeline);

		// transition backbuffer to present
		backbuffer->transition(&commandlist, graphics::e_resource_state::present);

		commandlist.submit(&queue);

		swapchain.present(present_args);
	}

	// cleanup..
	// eh...
}