#pragma once
#include "influx_graphics/rootsignature.h"
#include "influx_graphics/d3d12/dx12_base.h"

struct ID3D12RootSignature;

namespace influx::graphics
{
	class dx12_rootsignature final : public rootsignature
	{
		ID3D12RootSignature* mpdx_rootsignature;
	private:
		dx12_rootsignature(ID3D12RootSignature* rootsignature, const rootsignature_desc& desc, const umap<string, uint32>& name_to_paramidx);
		virtual void release_impl(device*) override;
		friend class dx12_device;
	};
}