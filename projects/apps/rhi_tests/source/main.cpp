
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
	
	graphics::swapchain_desc swpchain_desc{};
	swpchain_desc.m_dimensions = window->get_dimensions(platform::window::e_space::client);
	swpchain_desc.m_format = graphics::e_format::rgba8;
	swpchain_desc.m_num_buffers = 3u;
	graphics::swapchain* swapchain = device->create_swapchain(queue, *window, swpchain_desc);

	static rendergraph::rgrendertarget_id rt_ids[3u]{};
	static int buffer_index = 0u;

	while (true)
	{
		rendergraph::rendergraph graph{ device };
		graph.import_texture(RGNAME_IDX("swapchain_buffer", 0u), swapchain->get_backbuffer_resource(0u));
		graph.import_texture(RGNAME_IDX("swapchain_buffer", 1u), swapchain->get_backbuffer_resource(1u));
		graph.import_texture(RGNAME_IDX("swapchain_buffer", 2u), swapchain->get_backbuffer_resource(2u));

		buffer_index = swapchain->acquire_backbuffer();

		graph.add_pass(
			[](rendergraph::rgpass_builder& builder)
			{
				rendergraph::rgaccess access{};
				rt_ids[buffer_index] = builder.write_rendertarget(RGNAME_IDX("swapchain_buffer", buffer_index), access);
			},
			[&graph](rendergraph::rgpass_context& ctx)
			{
				// get the descriptor handles and do something to them
				// influx::graphics::descriptor_handle rtv_handle = ctx.get_rtv(rt_id);
			});

		graph.build();

		commandlist->start(device);
		graph.execute(commandlist);
		swapchain->get_current_backbuffer_resource()->transition(commandlist, graphics::e_resource_state::present);
		commandlist->end();

		commandlist->submit(queue);
		commandlist->wait_for_completion();

		swapchain->present({});
	}
}