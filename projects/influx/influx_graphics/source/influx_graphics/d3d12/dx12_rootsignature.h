#pragma once
#include "influx_graphics/rootsignature.h"

struct ID3D12RootSignature;

namespace influx::graphics
{
	class dx12_rootsignature final : public rootsignature
	{
	public:
		dx12_rootsignature(ID3D12RootSignature* rootsignature, const rootsignature_desc& desc, const umap<string, uint32>& name_to_paramidx);

	private:
		ID3D12RootSignature* mpdx_rootsignature;
	};
}