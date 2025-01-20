
#include "core/basetypes.h"

// SDK 1.614.1
extern "C" { __declspec(dllexport) extern const influx::uint32 D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

#include "influx_platform/window.h"
#include "influx_graphics/device.h"
#include "rendergraph.h"

int main()
{
	using namespace influx;
	platform::window_desc window_desc{};
	window_desc.m_dimensions = { 640u, 480u };
	window_desc.m_name = "rendergraph";

	platform::window* window = platform::window::create(window_desc);
	graphics::device* device = graphics::device::create(graphics::e_api_type::dx12);
	graphics::queue* queue = device->create_queue();
	graphics::commandlist* commandlist = device->create_graphics_commandlist();

	rendergraph::rendergraph graph{ device };
	auto* clear_pass = graph.add_pass([](rendergraph::rgpass_context& ctx)
	{
		
	});;

	auto* present_pass = graph.add_pass([](rendergraph::rgpass_context& ctx)
	{
		
	});

	graph.build();

	commandlist->start(device);
	graph.execute(commandlist);
	commandlist->end();

	while (true)
	{
		commandlist->submit(queue);
		commandlist->wait_for_completion();
	}
}