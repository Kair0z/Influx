#pragma once
#include "influx_graphics/commandallocator.h"

struct ID3D12CommandAllocator;

namespace influx::graphics
{
	class dx12_command_allocator final : public command_allocator
	{
	public:
		dx12_command_allocator(ID3D12CommandAllocator* allocator);

	private:
		ID3D12CommandAllocator* mpdx_allocator;
	};
}