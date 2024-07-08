#pragma once
#include "influx_graphics/resource.h"

struct ID3D12Resource;

namespace influx::graphics
{
	class dx12_resource final : public resource
	{
		virtual void* map(const map_args& args) override;
		virtual void unmap(const map_args& args) override;

	public:
		dx12_resource(ID3D12Resource* resource, const tex2D_desc& desc);
		dx12_resource(ID3D12Resource* resource, const buffer_desc& desc);

	private:
		ID3D12Resource* mpdx_resource;

	protected:
#if _DEBUG
		virtual void set_name_impl(const string& new_name) override;
#endif
	};
}