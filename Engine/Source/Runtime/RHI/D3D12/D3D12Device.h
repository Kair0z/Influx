#pragma once
#include "Runtime/RHI/RHIDevice.h"

namespace Influx
{
	class D3D12Device final : public RHIDevice
	{
	public:
		D3D12Device() = default;
		~D3D12Device() = default;
		D3D12Device(const D3D12Device&) = delete;
		D3D12Device(D3D12Device&&) = delete;
		D3D12Device& operator=(const D3D12Device&) = delete;
		D3D12Device& operator=(D3D12Device&&) = delete;

	private:

	};
}


