#pragma once
#include "influx_graphics/commandallocator.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include "d3dx12.h"

namespace influx::graphics
{
	class dx12_command_allocator final : public command_allocator
	{
	public:
		dx12_command_allocator(ID3D12CommandAllocator* allocator)
			: mpdx_allocator{allocator}
		{
			mp_native = mpdx_allocator = allocator;
		}

	private:
		ID3D12CommandAllocator* mpdx_allocator;
	};
}