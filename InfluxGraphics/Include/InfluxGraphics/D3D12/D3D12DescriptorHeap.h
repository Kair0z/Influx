#pragma once

#ifndef __GR_D3D12_DESCRIPTORHEAP_H_
#define __GR_D3D12_DESCRIPTORHEAP_H_

#include "InfluxGraphics/Common.h"

#include "InfluxGraphics/RHIDescriptorHeap.h"
#include "D3D12.h"

namespace Influx::Graphics
{
	/* D3D12DescriptorHeap */
	class D3D12DescriptorHeap final : public RHIDescriptorHeap
	{
		ID3D12DescriptorHeap* mp_dxDescriptorHeap;
		List<uint64> m_occupiedSlotIndices;
		uint64 m_descriptorStride;

	public:
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint64 slot);
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint64 slot);

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle();
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle();

		bool GetHandles(uint64 slot, D3D12_CPU_DESCRIPTOR_HANDLE& out_cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE& out_gpuHandle);
		bool GetHandles(D3D12_CPU_DESCRIPTOR_HANDLE& out_cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE& out_gpuHandle);

		ID3D12DescriptorHeap* GetDxDescriptorHeap() const;
		bool IsSlotFree(uint64 slot) const;
		uint64 GetFirstFreeSlot() const;

		constexpr static D3D12_CPU_DESCRIPTOR_HANDLE NullCPUHandle() { return {}; };
		constexpr static D3D12_GPU_DESCRIPTOR_HANDLE NullGPUHandle() { return {}; };

	private:
		D3D12DescriptorHeap(const ERHIDescriptorType type, uint64 numDescriptors, bool isShaderVisible);
		virtual ~D3D12DescriptorHeap();

		friend class D3D12Device;
	};
}

#endif