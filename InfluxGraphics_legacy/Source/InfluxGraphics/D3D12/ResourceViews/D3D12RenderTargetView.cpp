#include "InfluxGraphics/D3D12/ResourceViews/D3D12RenderTargetView.h"

namespace influx::Graphics
{
	D3D12_CPU_DESCRIPTOR_HANDLE D3D12RenderTargetView::GetDxCPUHandle() const
	{
		return m_dxCpuHandle;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE D3D12RenderTargetView::GetDxGPUHandle() const
	{
		return m_dxGpuHandle;
	}
}
