
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
	using namespace influx::rendergraph;

	platform::window_desc window_desc{};
	window_desc.m_dimensions = { 640u, 480u };
	window_desc.m_name = "rendergraph";

	platform::window* window = platform::window::create(window_desc);
	graphics::device* device = graphics::device::create(graphics::e_api_type::dx12);
	graphics::queue* queue = device->create_queue();
	graphics::commandlist* commandlist = device->create_graphics_commandlist();
	
	graphics::swapchain_desc swpchain_desc{};
	swpchain_desc.m_dimensions = window->get_dimensions(platform::window::e_space::client);
	swpchain_desc.m_format = graphics::e_format::rgba8;
	swpchain_desc.m_num_buffers = 3u;
	graphics::swapchain* swapchain = device->create_swapchain(queue, *window, swpchain_desc);

	static int buffer_index = 0u;

	while (true)
	{
		influx::rendergraph::rendergraph graph{ device };

		auto res = swapchain->get_current_backbuffer_resource();
		if (res.is_unex()) continue;

		graph.import_texture(RGNAME("backbuffer"), res.get());

		// clear backbuffer pass
		graph.add_pass(e_rgpass_type::graphics,
			[](rgpass_builder& builder)
			{
				rgaccess access{};
				access.m_load = e_rg_load::clear;
				access.m_store = e_rg_store::preserve;
				builder.write_rendertarget(RGNAME("backbuffer"), access);
			},
			[&graph](rgpass_context& ctx)
			{

			});

		commandlist->start(device);

		graph.build();
		graph.execute(commandlist);

		// transition backbuffer to present
		res.get()->transition(commandlist, graphics::e_resource_state::present);
		commandlist->end();
		commandlist->submit(queue);
		commandlist->wait_for_completion();

		swapchain->present({ .m_vsync = false });
	}
}