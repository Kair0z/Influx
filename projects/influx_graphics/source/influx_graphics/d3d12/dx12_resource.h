#pragma once
#include "influx_graphics/resource.h"

struct ID3D12Resource;

namespace influx::graphics
{
	class dx12_resource final : public resource
	{
	public:
		dx12_resource(ID3D12Resource* resource, const tex2D_desc& desc);

	private:
		ID3D12Resource* mpdx_resource;
	};
}