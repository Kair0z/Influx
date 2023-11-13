#pragma once

#ifndef __GR_D3D12_PIPELINELAYOUT_H_
#define __GR_D3D12_PIPELINELAYOUT_H_

#include "InfluxGraphics/RHIPipelineLayout.h"
#include "D3D12.h"

namespace influx::Graphics
{
	/* 
	* D3D12GraphicsPipelineLayout
	* D3D12GraphicsRootSignature 
	*/
	class D3D12GraphicsPipelineLayout final : public RHIGraphicsPipelineLayout
	{
		friend class D3D12Device;
		D3D12GraphicsPipelineLayout() = default;

		ID3D12RootSignature* mp_dxRootSignature;

	public:
		ID3D12RootSignature* GetDxRootSignature() const;
	};
}

#endif