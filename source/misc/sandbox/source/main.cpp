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
	auto device = rhi::device::create(rhi::device_desc{}).get();

	// create queue
	rhi::queue_desc queue_desc{
		.m_device = device.m_native_object,
		.m_type = rhi::e_queue_type::graphics,
		.m_priority = 0 };
	rhi::queue queue = device.create(queue_desc).get();

	// create swapchain
	rhi::swapchain_desc swpdesc{};
	swpdesc.m_dimensions = windesc.m_dimensions;
	swpdesc.m_window = window->get_platform_handle();
	swpdesc.m_num_buffers = 3u;
	swpdesc.m_device = &device;
	swpdesc.m_queue = &queue;
	rhi::swapchain swapchain = device.create(swpdesc).get();

	// create a descheap
	rhi::descheap_desc heap_desc{};
	heap_desc.m_device = device.m_native_object;
	heap_desc.m_num_descriptors = 3u;
	heap_desc.m_type = rhi::e_descriptor_heap_type::rtv;
	auto rtv_heap = device.create(heap_desc).get();

	// create a commandlist
	auto allocator = device.create(rhi::commandallocator_desc{}).get();
	rhi::commandlist_desc cmdlist_desc{};
	cmdlist_desc.m_allocator = allocator.m_native_object;
	cmdlist_desc.m_device = device.m_native_object;
	auto cmd = device.create(cmdlist_desc).get();

	cmd.start(allocator);

	rhi::texture2D backbuffer = swapchain.get_backbuffer_resource().get();
	backbuffer.transition(cmd, rhi::e_resource_state::rendertarget);

	// create the render target view
	rhi::descriptor backbuffer_rtv = rtv_heap.get_cpu_descriptor(0).get();
	device.create_rtv(backbuffer, backbuffer_rtv);

	math::float4 colour = { 1,0,0,1 };
	cmd.clear_rtv(backbuffer_rtv, { colour });

	backbuffer.transition(cmd, rhi::e_resource_state::present);
	cmd.end();

	queue.submit({ &cmd });

	bool is_exit = false;
	while (!is_exit)
	{
		window->poll_events(is_exit);
		swapchain.present();
	}
}