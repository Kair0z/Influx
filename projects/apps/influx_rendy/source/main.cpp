
#include "influx_graphics.h"

extern "C" { __declspec(dllexport) extern const uint32_t D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

using namespace influx;

#include <thread>

int main(int argc, char** argv)
{
	using namespace influx;

	graphics::device* device = graphics::device::create(graphics::e_api_type::dx12);
	
	graphics::commandbuffer* buffer = device->create_commandbuffer();

	device->submit(buffer);

	while (!buffer->is_finished_gpu())
	{
		buffer->submit();
	}
}