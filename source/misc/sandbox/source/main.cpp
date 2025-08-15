// influx::rhi
#include "influx_rhi.h"

// influx::platform
#include "influx_platform/window.h"

int main()
{
	using namespace influx;

	// create a window
	platform::window_desc windesc{};
	windesc.m_dimensions = { 640u, 480u };
	windesc.m_name = "sandbox";
	platform::window* window = platform::window::create(windesc);

	// create a device
	rhi::device_desc device_desc{};
	device_desc.m_debug = false;
	rhi::device device = rhi::device::create(device_desc).get();

	// create queue
	rhi::queue queue = device.create(rhi::queue_desc::default_graphics()).get();

	// create swapchain
	rhi::swapchain_desc swpdesc{};
	swpdesc.m_dimensions = windesc.m_dimensions;
	swpdesc.m_window = window->get_platform_handle();
	swpdesc.m_num_buffers = 3u;
	swpdesc.m_device = &device;
	swpdesc.m_queue = &queue;
	swpdesc.m_own_descriptors = true; // swapchain owns an rtv descriptor heap
	rhi::swapchain swapchain = device.create(swpdesc).get();

	// create a commandlist
	auto cmd = device.create(rhi::commandlist_desc::default_graphics()).get();

	math::float4 colour = { 1,0,0,1 };
	bool is_exit = false;
	uint32 frame = 0u;
	while (!is_exit)
	{
		window->poll_events(is_exit);

		swapchain.resize(window->get_dimensions(platform::window::e_space::client));

		cmd.start(device);

		rhi::texture2D backbuffer = swapchain.get_backbuffer_resource().get();
		backbuffer.transition(cmd, rhi::e_resource_state::rendertarget);

		rhi::descriptor backbuffer_rtv = swapchain.get_or_create_backbuffer_rtv(device).get();
		cmd.clear_rtv(backbuffer_rtv, { colour });
		backbuffer.transition(cmd, rhi::e_resource_state::present);
		cmd.end();
		cmd.submit(queue);
		cmd.wait_for_finish();

		swapchain.present();
		++frame;
	}
}