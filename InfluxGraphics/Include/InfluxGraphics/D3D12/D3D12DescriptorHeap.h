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
		constexpr static D3D12_CPU_DESCRIPTOR_HANDLE NullCPUHandle() { return {}; };
		constexpr static D3D12_GPU_DESCRIPTOR_HANDLE NullGPUHandle() { return {}; };

		ID3D12DescriptorHeap* mp_dxDescriptorHeap;
		uint64 m_descriptorStride;

		struct SlotHolder final
		{
		private:
			List<uint64> m_occupiedSlotIndices{};
			
		public:
			void SetSlotOccupied(uint64 slot)
			{
				m_occupiedSlotIndices.push_back(slot);
			}

			bool IsSlotFree(uint64 slot) const
			{
				return std::find(m_occupiedSlotIndices.cbegin(), m_occupiedSlotIndices.cend(), slot) == m_occupiedSlotIndices.cend();
			}

			uint64 GetFirstFreeSlot(uint64 totalNumDescriptors) const
			{
				for (uint64 i = 0; i < totalNumDescriptors; ++i)
				{
					if (IsSlotFree(i)) return i;
				}

				FLX_ASSERT(false); // no free slots?
				return std::numeric_limits<uint64>::max();
			}
		};

		SlotHolder m_slotHolder_gpu;
		SlotHolder m_slotHolder_cpu;

	public:
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint64 slot);
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint64 slot);

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle();
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle();

		bool GetHandles(D3D12_CPU_DESCRIPTOR_HANDLE& out_cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE& out_gpuHandle);

		bool GetCPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE& out_handle);
		bool GetGPUHandle(D3D12_GPU_DESCRIPTOR_HANDLE& out_handle);

		bool IsSlotFreeCPU(uint64 slot) const;
		bool IsSlotFreeGPU(uint64 slot) const;

		uint64 GetFirstFreeSlotCPU() const;
		uint64 GetFirstFreeSlotGPU() const;

		ID3D12DescriptorHeap* GetDxDescriptorHeap() const;

		
	private:
		D3D12DescriptorHeap(const ERHIResourceViewType type, uint64 numDescriptors, bool isShaderVisible);
		virtual ~D3D12DescriptorHeap();

		friend class D3D12Device;
	};
}

#endif