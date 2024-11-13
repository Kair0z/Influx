#include "graphics_pch.h"
#include "dx12_headers.h"
#include "dx12_rootsignature.h"

namespace influx::graphics
{
	dx12_rootsignature::dx12_rootsignature(ID3D12RootSignature* rootsignature, const rootsignature_desc& desc, const umap<string, uint32>& name_to_paramidx)
		: rootsignature(desc, name_to_paramidx)
	{
		mp_native = mpdx_rootsignature = rootsignature;
	}

	void dx12_rootsignature::release()
	{
		mpdx_rootsignature->Release();
	}
}

