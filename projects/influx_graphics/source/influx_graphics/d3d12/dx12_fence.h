#pragma once 
#include "influx_graphics/fence.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include "d3dx12.h"

namespace influx::graphics
{
	class dx12_fence final : public fence
	{
	public:
		dx12_fence(ID3D12Fence* fence)
		{
			mp_native = mpdx_fence = fence;
		}

	private:
		ID3D12Fence* mpdx_fence;
	};
}