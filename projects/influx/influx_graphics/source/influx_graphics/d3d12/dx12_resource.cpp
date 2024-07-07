#include "graphics_pch.h"
#include "influx_graphics/d3d12/dx12_resource.h"
#include "dx12_headers.h"

namespace influx::graphics
{
	dx12_resource::dx12_resource(ID3D12Resource* resource, const tex2D_desc& desc)
		: resource(desc)
	{
		mp_native = mpdx_resource = resource;
	}

#if _DEBUG
	void dx12_resource::set_name_impl(const string& new_name)
	{
		mpdx_resource->SetName(to_wstring(new_name).c_str());
	}
#endif
}