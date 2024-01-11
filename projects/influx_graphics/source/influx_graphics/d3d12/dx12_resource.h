#pragma once
#include "influx_graphics/resource.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include "d3dx12.h"

namespace influx::graphics
{
	class dx12_resource final : public resource
	{
	public:
		dx12_resource(ID3D12Resource* resource, const tex2D_desc& desc)
			: resource(desc)
		{
			mp_native = mpdx_resource = resource;
		}

	private:
		ID3D12Resource* mpdx_resource;
	};
}