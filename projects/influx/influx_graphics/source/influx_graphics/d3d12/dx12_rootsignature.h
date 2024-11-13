#pragma once
#include "influx_graphics/rootsignature.h"
#include "influx_graphics/d3d12/dx12_base.h"

struct ID3D12RootSignature;

namespace influx::graphics
{
	class dx12_rootsignature final : public rootsignature
	{
	public:
		dx12_rootsignature(ID3D12RootSignature* rootsignature, const rootsignature_desc& desc, const umap<string, uint32>& name_to_paramidx);

	private:
		ID3D12RootSignature* mpdx_rootsignature;

		virtual void release() override;
	};
}