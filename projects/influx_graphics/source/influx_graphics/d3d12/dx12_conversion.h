#pragma once

#include "influx_graphics.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include "d3dx12.h"

namespace influx::graphics
{
	D3D12_COMMAND_LIST_TYPE convert(e_command_queue_type type)
	{
		switch (type)
		{
		case e_command_queue_type::graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
		case e_command_queue_type::compute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
		case e_command_queue_type::copy: return D3D12_COMMAND_LIST_TYPE_COPY;
		default: return D3D12_COMMAND_LIST_TYPE_DIRECT;
		}
	}

	DXGI_FORMAT convert(e_format format)
	{
		switch (format)
		{
		case e_format::rgba8: return DXGI_FORMAT_R8G8_UNORM;
		default: return DXGI_FORMAT_R8G8_UNORM;
		}
	}

	D3D12_RESOURCE_FLAGS convert(e_resource_flags flags)
	{
		switch (flags)
		{
		case e_resource_flags::none: return D3D12_RESOURCE_FLAG_NONE;
		default: return D3D12_RESOURCE_FLAG_NONE;
		}
	}
}