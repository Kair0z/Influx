
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

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::GetDescriptorHandle(uint64 slot)
	{
		if (!IsSlotFree(slot))
		{
			// Slot is in the Freelist. Thus there's no descriptor here...
			return NullDescriptorHandle();
		}

		D3D12_CPU_DESCRIPTOR_HANDLE handle = GetDxDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += (slot * m_descriptorStride);

		return handle;
	}

	ID3D12DescriptorHeap* D3D12DescriptorHeap::GetDxDescriptorHeap() const
	{
		return mp_dxDescriptorHeap;
	}

	bool D3D12DescriptorHeap::IsSlotFree(uint64 slot) const
	{
		return std::find(m_occupiedSlotIndices.cbegin(), m_occupiedSlotIndices.cend(), slot) == m_occupiedSlotIndices.cend();
	}

	size_t D3D12DescriptorHeap::GetFirstFreeSlot() const
	{
		for (uint64 i = 0; i < GetNumDescriptors(); ++i)
		{
			if (IsSlotFree(i)) return i;
		}

		FLX_ASSERT(false); // no free slots?
		return std::numeric_limits<size_t>::max();
	}
}