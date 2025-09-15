#define USE_GRAPHICS	0
#define USE_RHI			1

#if USE_GRAPHICS
#include "influx_graphics/device.h"
// SDK 1.614.1
extern "C" { __declspec(dllexport) extern const influx::uint32 D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }
#endif

#if USE_RHI
#include "influx_rhi.h"
#endif

#include <iostream>
#include "core/basetypes.h"
#include "influx_platform/window.h"
using namespace influx;

#if USE_GRAPHICS
graphics::resource* create_texture(graphics::device* device, graphics::queue* queue)
{
	const uint32 arraysize = 6u;
	const uint32 size = 256u;
	const uint64 subresource_bytesize = size * size * sizeof(uint32);
	const uint64 bytesize = arraysize * subresource_bytesize;

	graphics::cubemap_desc desc{};
	desc.m_dimensions.x = desc.m_dimensions.y = size;
	desc.m_format = graphics::e_format::rgba8;
	graphics::resource* result = device->create_resource(desc);
	graphics::resource* upload_resource = device->create_upload_resource(result);

	// map to upload
	{
		graphics::map_args args{ .m_subres = 0u, .m_begin = 0u, .m_end = bytesize };
		upload_resource->map([arraysize, subresource_bytesize](void* target)
		{
			uint8* byte = reinterpret_cast<uint8*>(target);
			for (uint64 a = 0u; a < arraysize; ++a)
			{
				const uint64 base = a * subresource_bytesize;
				for (uint64 i = 0u; i < subresource_bytesize; ++i)
				{
					*(byte + i + base) = (a % 2 == 0) ? 255 : 0;
				}
			}
		}, args);
	}

	// upload -> gpu
	{
		graphics::commandlist& commandlist = *device->create_graphics_commandlist();
		commandlist.start(device);

		upload_resource->transition(&commandlist, graphics::e_resource_state::copy_src);
		result->transition(&commandlist, graphics::e_resource_state::copy_dst);
		graphics::copy_texture_args args{};
		commandlist.copy_texture(upload_resource, result, args);

		commandlist.end();
		commandlist.submit(queue);
		commandlist.wait_for_completion();
	}
	
	return result;
}
int graphics_main()
{
	graphics::device* device = graphics::device::create(graphics::e_api_type::dx12);
	graphics::queue* queue = device->create_queue();
	graphics::commandlist* commandlist = device->create_graphics_commandlist();

	platform::window_desc window_desc{};
	window_desc.m_dimensions = { 640u, 480u };
	window_desc.m_name = "renderer";

	static constexpr uint32 num_windows = 1u;
	platform::window* windows[num_windows] =
	{
		platform::window::create(window_desc.set_name("A"))
	};

	graphics::swapchain_desc swap_desc{};
	graphics::swapchain* swapchains[num_windows]
	{
		device->create_swapchain(queue, *windows[0], swap_desc)
	};

	graphics::resource* texture = create_texture(device, queue);

	bool is_quit = false;
	graphics::present_args pres_args{};
	while (!is_quit)
	{
		commandlist->start(device);
		for (uint32 i = 0u; i < num_windows; ++i)
		{
			swapchains[i]->acquire_backbuffer();
			graphics::resource* backbuffer = swapchains[i]->get_current_backbuffer_resource().get();

			texture->transition(commandlist, graphics::e_resource_state::copy_src);
			backbuffer->transition(commandlist, graphics::e_resource_state::copy_dst);
			commandlist->copy_texture(texture, backbuffer);
			backbuffer->transition(commandlist, graphics::e_resource_state::present);
			windows[i]->poll_events(is_quit);
		}
		commandlist->end();
		commandlist->submit(queue);
		commandlist->wait_for_completion();

		device->get_memory_info().get();

		for (uint32 i = 0u; i < num_windows; ++i)
		{
			swapchains[i]->present(pres_args);
		}
	}
	return 0;
}
#endif

#if USE_RHI
int rhi_main()
{
	// make a platform window
	platform::window_desc window_desc{};
	window_desc.m_dimensions = { 640u, 480u };
	window_desc.m_name = "renderer";
	platform::window* window = platform::window::create(window_desc.set_name("influx_rhi"));

	rhi::device_create_args dev_args{};
	dev_args.m_debug = true;
	rhi::device dev				= rhi::create_device(dev_args).get();
	rhi::queue queue			= dev.create(rhi::queue::default_graphics()).get();
	rhi::commandlist cmdlist	= dev.create(rhi::commandlist::default_graphics()).get();

	rhi::swapchain_create_args swapchain_args{};
	{
		swapchain_args.m_platform_instance	= platform::platform::get_current_instance();
		swapchain_args.m_window				= window->get_platform_handle();
		swapchain_args.m_dimensions			= window->get_dimensions();
		swapchain_args.m_queue				= queue.m_native_object;
		swapchain_args.m_format				= rhi::pixelformat::rgba_8_unorm();
	}
	rhi::swapchain swapchain = dev.create(swapchain_args).get();

	bool is_quit = false;
	while (!is_quit)
	{
		window->poll_events(is_quit);
		if (is_quit) break;

		swapchain.acquire_backbuffer(dev.m_native_object);

		cmdlist.start(dev);
		rhi::texture& backbuffer = swapchain.get_backbuffer_resource().get();
		cmdlist.clear_texture(dev, backbuffer, rhi::clear::colour({1,0,0,1}));
		cmdlist.end();
		cmdlist.submit(queue);

		rhi::present_args args{};
		args.m_device = dev.m_native_object;
		args.m_present_queue = queue.m_native_object;
		swapchain.present(args);

		cmdlist.wait_for_finish();
	}
	return 0u;
}
#endif

int main()
{
#if USE_RHI
	rhi_main();
#endif
#if USE_GRAPHICS
	graphics_main();
	// rhi_main();
#endif
}