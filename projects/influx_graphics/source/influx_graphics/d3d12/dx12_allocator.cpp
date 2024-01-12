#include "graphics_pch.h"
#include "influx_graphics/d3d12/dx12_allocator.h"
#include "dx12_headers.h"


namespace influx::graphics
{
	dx12_command_allocator::dx12_command_allocator(ID3D12CommandAllocator* allocator)
		: mpdx_allocator{ allocator }
	{
		mp_native = mpdx_allocator = allocator;
	}
}