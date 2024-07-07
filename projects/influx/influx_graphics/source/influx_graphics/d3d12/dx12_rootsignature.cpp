#include "graphics_pch.h"
#include "dx12_headers.h"
#include "dx12_rootsignature.h"

namespace influx::graphics
{
	dx12_rootsignature::dx12_rootsignature(ID3D12RootSignature* rootsignature, const rootsignature_desc& desc)
		: rootsignature(desc)
	{
		mp_native = mpdx_rootsignature = rootsignature;
	}
}

