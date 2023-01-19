
#include "InfluxGraphics/Common.h"
#include "InfluxGraphics/D3D12/D3D12DescriptorHeap.h"

namespace Influx::Graphics
{
	D3D12DescriptorHeap::D3D12DescriptorHeap(const ERHIResourceViewType type, uint64 numDescriptors, bool isShaderVisible)
		: RHIDescriptorHeap(type, numDescriptors, isShaderVisible)
	{

	}

	D3D12DescriptorHeap::~D3D12DescriptorHeap()
	{
		//D3D12API::SafeRelease(DxDescriptorHeap);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::GetCPUHandle(uint64 slot)
	{
		if (!IsSlotFreeGPU(slot))
		{
			// Slot is in the Freelist. Thus there's no descriptor here...
			return NullCPUHandle();
		}

		m_slotHolder_cpu.SetSlotOccupied(slot);

		D3D12_CPU_DESCRIPTOR_HANDLE handle = GetDxDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += (slot * m_descriptorStride);

		return handle;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::GetGPUHandle(uint64 slot)
	{
		if (!IsSlotFreeGPU(slot))
		{
			// Slot is in the Freelist. Thus there's no descriptor here...
			return NullGPUHandle();
		}

		m_slotHolder_gpu.SetSlotOccupied(slot);

		D3D12_GPU_DESCRIPTOR_HANDLE handle = GetDxDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
		handle.ptr += (slot * m_descriptorStride);

		return handle;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::GetCPUHandle()
	{
		return GetCPUHandle(GetFirstFreeSlotCPU());
	}

	D3D12_GPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::GetGPUHandle()
	{
		return GetGPUHandle(GetFirstFreeSlotGPU());
	}

	bool D3D12DescriptorHeap::GetHandles(D3D12_CPU_DESCRIPTOR_HANDLE& out_cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE& out_gpuHandle)
	{
		out_cpuHandle = GetCPUHandle();
		out_gpuHandle = GetGPUHandle();

		if (out_gpuHandle.ptr == NullGPUHandle().ptr|| out_cpuHandle.ptr == NullCPUHandle().ptr)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	ID3D12DescriptorHeap* D3D12DescriptorHeap::GetDxDescriptorHeap() const
	{
		return mp_dxDescriptorHeap;
	}

	bool D3D12DescriptorHeap::IsSlotFreeCPU(uint64 slot) const
	{
		return m_slotHolder_cpu.IsSlotFree(slot);
	}

	bool D3D12DescriptorHeap::IsSlotFreeGPU(uint64 slot) const
	{
		return m_slotHolder_gpu.IsSlotFree(slot);
	}

	uint64 D3D12DescriptorHeap::GetFirstFreeSlotCPU() const
	{
		return m_slotHolder_cpu.GetFirstFreeSlot(GetNumDescriptors());
	}

	uint64 D3D12DescriptorHeap::GetFirstFreeSlotGPU() const
	{
		return m_slotHolder_gpu.GetFirstFreeSlot(GetNumDescriptors());
	}
}