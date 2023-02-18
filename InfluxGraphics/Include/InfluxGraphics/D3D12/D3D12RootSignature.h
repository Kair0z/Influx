#pragma once

#ifndef __GR_D3D12_ROOTSIGNATURE_H_
#define __GR_D3D12_ROOTSIGNATURE_H_

#include "InfluxGraphics/RHIRootSignature.h"
#include "D3D12.h"

namespace Influx::Graphics
{
	/* D3D12RootSignature */
	class D3D12RootSignature final : public RHIRootSignature
	{
		friend class D3D12Device;
		D3D12RootSignature() = default;

		ID3D12RootSignature* mp_dxRootSignature;
	};
}

#endif