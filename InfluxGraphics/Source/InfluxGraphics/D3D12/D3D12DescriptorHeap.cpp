
#include "InfluxGraphics/Common.h"
#include "InfluxGraphics/D3D12/D3D12DescriptorHeap.h"

namespace Influx::Graphics
{
	D3D12DescriptorHeap::D3D12DescriptorHeap(const ERHIDescriptorType type, uint64 numDescriptors, bool isShaderVisible)
		: RHIDescriptorHeap(type, numDescriptors, isShaderVisible)
	{

	}

	D3D12DescriptorHeap::~D3D12DescriptorHeap()
	{
		//D3D12API::SafeRelease(DxDescriptorHeap);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::GetCPUHandle(uint64 slot)
	{
		if (!IsSlotFree(slot))
		{
			// Slot is in the Freelist. Thus there's no descriptor here...
			return NullCPUHandle();
		}

		D3D12_CPU_DESCRIPTOR_HANDLE handle = GetDxDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += (slot * m_descriptorStride);

		return handle;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::GetGPUHandle(uint64 slot)
	{
		if (!IsSlotFree(slot))
		{
			// Slot is in the Freelist. Thus there's no descriptor here...
			return NullGPUHandle();
		}

		D3D12_GPU_DESCRIPTOR_HANDLE handle = GetDxDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
		handle.ptr += (slot * m_descriptorStride);

		return handle;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::GetCPUHandle()
	{
		return GetCPUHandle(GetFirstFreeSlot());
	}

	D3D12_GPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::GetGPUHandle()
	{
		return GetGPUHandle(GetFirstFreeSlot());
	}

	bool D3D12DescriptorHeap::GetHandles(uint64 slot, D3D12_CPU_DESCRIPTOR_HANDLE& out_cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE& out_gpuHandle)
	{
		if (!IsSlotFree(slot))
		{
			// Slot is in the Freelist. Thus there's no descriptor here...
			out_cpuHandle = NullCPUHandle();
			out_gpuHandle = NullGPUHandle();
			return false;
		}

		out_cpuHandle = GetDxDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
		out_gpuHandle = GetDxDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
		out_cpuHandle.ptr += (slot * m_descriptorStride);
		out_gpuHandle.ptr += (slot * m_descriptorStride);

		return true;
	}

	bool D3D12DescriptorHeap::GetHandles(D3D12_CPU_DESCRIPTOR_HANDLE& out_cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE& out_gpuHandle)
	{
		return GetHandles(GetFirstFreeSlot(), out_cpuHandle, out_gpuHandle);
	}

	ID3D12DescriptorHeap* D3D12DescriptorHeap::GetDxDescriptorHeap() const
	{
		return mp_dxDescriptorHeap;
	}

	bool D3D12DescriptorHeap::IsSlotFree(uint64 slot) const
	{
		return std::find(m_occupiedSlotIndices.cbegin(), m_occupiedSlotIndices.cend(), slot) == m_occupiedSlotIndices.cend();
	}

	uint64 D3D12DescriptorHeap::GetFirstFreeSlot() const
	{
		for (uint64 i = 0; i < GetNumDescriptors(); ++i)
		{
			if (IsSlotFree(i)) return i;
		}

		FLX_ASSERT(false); // no free slots?
		return std::numeric_limits<size_t>::max();
	}
}