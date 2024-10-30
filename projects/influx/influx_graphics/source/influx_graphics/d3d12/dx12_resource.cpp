#include "graphics_pch.h"
#include "influx_graphics/d3d12/dx12_resource.h"
#include "dx12_headers.h"
#include "core/math/math.h"

namespace influx::graphics
{
	dx12_resource::dx12_resource(ID3D12Resource* resource, const tex2D_desc& desc)
		: resource(desc)
	{
		mp_native = mpdx_resource = resource;
		set_releasable(mpdx_resource);
	}

	dx12_resource::dx12_resource(ID3D12Resource* resource, const buffer_desc& desc)
		: resource(desc)
	{
		mp_native = mpdx_resource = resource;
	}

	void dx12_resource::on_set_name(const debug_name& name)
	{
		mpdx_resource->SetName(to_wstring(name.get()).c_str());
	}

	void* dx12_resource::map(const map_args& args)
	{
		void* target_ptr;
		D3D12_RANGE range{};
		range.Begin = args.m_begin;
		range.End = min(args.m_end, get_bytesize());
		HRESULT res = mpdx_resource->Map(args.m_subres, &range, &target_ptr);
		return target_ptr;
	}

	void dx12_resource::unmap(const map_args& args)
	{
		D3D12_RANGE range{};
		range.Begin = args.m_begin;
		range.End = min(args.m_end, get_bytesize());
		mpdx_resource->Unmap(args.m_subres, &range);
	}
}