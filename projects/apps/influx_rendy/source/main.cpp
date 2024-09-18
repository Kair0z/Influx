
#include "influx_graphics.h"
#include "influx_graphics/commandbuffer.h"

extern "C" { __declspec(dllexport) extern const uint32_t D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

using namespace influx;

#include <thread>

int main(int argc, char** argv)
{
	using namespace influx::graphics;

	graphics::device* device = graphics::device::create(graphics::e_api_type::dx12);
	
	// creates the command list + allocator
	graphics::commandbuffer* buffer = device->create_commandbuffer();
	
	// pushes commands
	buffer->push<cmd_begin_renderpass>();
	buffer->push<cmd_draw_instanced>();

	while (!buffer->is_finished_gpu())
	{
		buffer->submit();
	}
}