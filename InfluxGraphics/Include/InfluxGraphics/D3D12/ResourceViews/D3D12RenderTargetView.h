#pragma once

#ifndef __GR_D3D12_RENDERTARGETVIEW_H_
#define __GR_D3D12_RENDERTARGETVIEW_H_

#include "InfluxGraphics/RHIResourceViews/RHIRenderTargetView.h"
#include "D3D12.h"

namespace Influx::Graphics
{
	class D3D12RenderTargetView final : public RHIRenderTargetView
	{
		friend class D3D12Device;
		D3D12RenderTargetView(ERHIFormat rtvFormat, const Math::Vectoru2& dimensions, const RHIClearValue resourceClearValue) 
			: RHIRenderTargetView(rtvFormat, dimensions, resourceClearValue) {}

		D3D12_CPU_DESCRIPTOR_HANDLE m_dxCpuHandle{};
		D3D12_GPU_DESCRIPTOR_HANDLE m_dxGpuHandle{};

	public:
		D3D12_CPU_DESCRIPTOR_HANDLE GetDxCPUHandle() const;
		D3D12_GPU_DESCRIPTOR_HANDLE GetDxGPUHandle() const;

		
		virtual ~D3D12RenderTargetView() = default;
	};
}

#endif
