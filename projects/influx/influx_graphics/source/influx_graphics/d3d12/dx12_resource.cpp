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
	}

	dx12_resource::dx12_resource(ID3D12Resource* resource, const tex3D_desc& desc)
		: resource(desc)
	{
		mp_native = mpdx_resource = resource;
	}

	dx12_resource::dx12_resource(ID3D12Resource* resource, const cubemap_desc& desc)
		: resource(desc)
	{
		mp_native = mpdx_resource = resource;
	}

	dx12_resource::dx12_resource(ID3D12Resource* resource, const buffer_desc& desc)
		: resource(desc)
	{
		mp_native = mpdx_resource = resource;
	}

	void dx12_resource::set_name_impl(const debug_name& name)
	{
		mpdx_resource->SetName(to_wstring(name.get()).c_str());
	}

	bool dx12_resource::allows_uav() const
	{
		return (mpdx_resource->GetDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) != 0;
	}

	void dx12_resource::release_impl(device*)
	{
		mpdx_resource->Release();
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