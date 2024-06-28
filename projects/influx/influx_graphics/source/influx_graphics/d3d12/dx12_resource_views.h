#pragma once
#include "influx_graphics/resource_views.h"

// dx12 includes
#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include "d3dx12.h"

namespace influx::graphics
{
	class dx12_render_target_view : public render_target_view
	{
	public:
		dx12_render_target_view(D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle);

	private:
		D3D12_CPU_DESCRIPTOR_HANDLE m_dx_cpu_handle;
	};
}