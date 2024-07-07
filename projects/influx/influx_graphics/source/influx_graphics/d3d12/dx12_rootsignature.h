#pragma once
#include "influx_graphics/rootsignature.h"

struct ID3D12RootSignature;

namespace influx::graphics
{
	class dx12_rootsignature final : public rootsignature
	{
	public:
		dx12_rootsignature(ID3D12RootSignature* rootsignature, const rootsignature_desc& desc);

	private:
		ID3D12RootSignature* mpdx_rootsignature;
	};
}