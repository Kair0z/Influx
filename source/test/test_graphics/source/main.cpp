
#include "core/basetypes.h"

// SDK 1.614.1
extern "C" { __declspec(dllexport) extern const influx::uint32 D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

#include "influx_platform/window.h"
#include "influx_graphics/device.h"

using namespace influx;

graphics::resource* create_texture(graphics::device* device, graphics::queue* queue)
{
	const uint32 arraysize = 6u;
	const uint32 size = 256u;
	const uint64 subresource_bytesize = size * size * sizeof(uint32);
	const uint64 bytesize = arraysize * subresource_bytesize;

	graphics::cubemap_desc desc{};
	desc.m_dimensions.x = desc.m_dimensions.y = size;
	desc.m_arraysize = arraysize;
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

int main()
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
}