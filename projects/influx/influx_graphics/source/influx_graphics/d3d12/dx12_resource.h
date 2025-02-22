#pragma once

// influx::graphics
#include "influx_graphics/resource.h"

struct ID3D12Resource;

namespace influx::graphics
{
	class dx12_resource final : public resource
	{
		ID3D12Resource* mpdx_resource;

	private:
		dx12_resource(ID3D12Resource* resource, const tex2D_desc& desc);
		dx12_resource(ID3D12Resource* resource, const tex3D_desc& desc);
		dx12_resource(ID3D12Resource* resource, const buffer_desc& desc);
		virtual void release_impl(device*) override;
		friend class dx12_device;

		virtual void* map(const map_args& args) override;
		virtual void unmap(const map_args& args) override;

		virtual void set_name_impl(const debug_name& name) override;

		virtual bool allows_uav() const override;
	};
}