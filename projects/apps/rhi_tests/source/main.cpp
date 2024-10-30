#include "influx_graphics.h"


extern "C" { __declspec(dllexport) extern const uint32_t D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

using namespace influx;

void test_generic(graphics::device* device)
{
	graphics::queue* queue = device->create_queue();

	graphics::command_allocator* alloc = device->create_graphics_allocator();
	graphics::commandlist* list = device->create_graphics_command_list(alloc, nullptr);
	graphics::fence* fence = device->create_fence();

	queue->submit_commandlists({ list });

	queue->queue_signal(fence, 1u);

	wait_handle wait{};
	fence->wait_for_value(1u, wait);
}

void test_dx12()
{
	graphics::device* dx_device
		= graphics::device::create(graphics::e_api_type::dx12);

	test_generic(dx_device);

	delete dx_device;
}

void test_vulkan()
{
	graphics::device* vk_device
		= graphics::device::create(graphics::e_api_type::vulkan);

	test_generic(vk_device);
	
	delete vk_device;
}

int main()
{
	test_vulkan();
}