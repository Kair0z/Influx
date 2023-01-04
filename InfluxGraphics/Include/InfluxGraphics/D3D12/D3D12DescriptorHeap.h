#pragma once

#ifndef __GR_D3D12_DESCRIPTORHEAP_H_
#define __GR_D3D12_DESCRIPTORHEAP_H_

namespace Influx::Graphics
{
	/* D3D12DescriptorHeap */
	class D3D12DescriptorHeap final : public RHIDescriptorHeap
	{
		ID3D12DescriptorHeap* mp_dxDescriptorHeap;
		std::list<uint64> m_occupiedSlotIndices;
		uint64 m_descriptorStride;

	public:
		D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle(uint64 slot);
		ID3D12DescriptorHeap* GetDxDescriptorHeap() const;
		bool IsSlotFree(uint64 slot) const;
		uint64 GetFirstFreeSlot() const;

		constexpr static D3D12_CPU_DESCRIPTOR_HANDLE NullDescriptorHandle() { return {}; };

	private:
		D3D12DescriptorHeap(const ERHIDescriptorType type, uint64 numDescriptors, bool isShaderVisible);
		virtual ~D3D12DescriptorHeap();

		friend class D3D12Device;
	};
}

#endif