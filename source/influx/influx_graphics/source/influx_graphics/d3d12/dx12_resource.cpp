#include "graphics_pch.h"
#include "influx_graphics/d3d12/dx12_resource.h"
#include "dx12_headers.h"
#include "core/math/math.h"
#include "dx12_conversion.h"

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

	dx12_resource::dx12_resource(ID3D12Resource* resource, const acc_str_desc& desc)
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

	vector<resource::footprint> dx12_resource::get_footprints() const
	{
		uint32 num_subresources = get_num_subresources();
		vector<footprint> result{};
		result.resize(num_subresources);

		ID3D12Device* dxdevice = nullptr;
		HRESULT res = mpdx_resource->GetDevice(IID_PPV_ARGS(&dxdevice));
		dxdevice->Release();

		D3D12_RESOURCE_DESC desc = mpdx_resource->GetDesc();

		struct subresource_info final
		{
			explicit subresource_info(ID3D12Resource* resource, uint32 num_subresources)
				: m_resource{ resource }
			{
				m_footprints.resize(num_subresources);
				m_num_rows.resize(num_subresources);
				m_row_bytesizes.resize(num_subresources);
				m_bytesizes.resize(num_subresources);
			}

			D3D12_TEXTURE_COPY_LOCATION get_location(uint32 i)
			{
				D3D12_TEXTURE_COPY_LOCATION location{};
				location.PlacedFootprint = m_footprints[i];
				location.SubresourceIndex = i;
				location.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				location.pResource = m_resource;
				return location;
			}

			vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> m_footprints{};
			vector<uint32> m_num_rows{};
			vector<uint64> m_row_bytesizes{};
			vector<uint64> m_bytesizes{};
			ID3D12Resource* m_resource;
		};
		subresource_info infos{ mpdx_resource, num_subresources };

		dxdevice->GetCopyableFootprints(&desc,
			0u, // first subresource 
			num_subresources,
			0u, // base offset
			infos.m_footprints.data(),
			infos.m_num_rows.data(),
			infos.m_row_bytesizes.data(),
			infos.m_bytesizes.data());

		for (uint32 i = 0u; i < num_subresources; ++i)
		{
			result[i].m_bytesize = infos.m_bytesizes[i];
			result[i].m_depth = infos.m_footprints[i].Footprint.Depth;
			result[i].m_format = translate(infos.m_footprints[i].Footprint.Format);
			result[i].m_height = infos.m_footprints[i].Footprint.Height;
			result[i].m_num_rows = infos.m_num_rows[i];
			result[i].m_offset = infos.m_footprints[i].Offset;
			result[i].m_row_bytesize = infos.m_row_bytesizes[i];
			result[i].m_subresource_index = i;
			result[i].m_width = infos.m_footprints[i].Footprint.Width;
		}

		return result;
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