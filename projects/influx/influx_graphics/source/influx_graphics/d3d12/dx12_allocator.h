#pragma once
#include "influx_graphics/commandallocator.h"
#include "influx_graphics/d3d12/dx12_base.h"

struct ID3D12CommandAllocator;

namespace influx::graphics
{
	class dx12_command_allocator final 
		: public command_allocator
		, public dx12_base
	{
	public:
		dx12_command_allocator(ID3D12CommandAllocator* allocator);
		virtual ~dx12_command_allocator();

	private:
		ID3D12CommandAllocator* mpdx_allocator;
	};
}