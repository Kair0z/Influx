#include "graphics_pch.h"
#include "influx_graphics/d3d12/dx12_resource_views.h"
#include "dx12_headers.h"

namespace influx::graphics
{
	dx12_render_target_view::dx12_render_target_view(D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle)
		: render_target_view()
		, m_dx_cpu_handle{ cpu_handle }
	{
		mp_native = reinterpret_cast<void*>(m_dx_cpu_handle.ptr);
	}
}