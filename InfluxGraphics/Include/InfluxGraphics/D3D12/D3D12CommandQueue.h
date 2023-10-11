#pragma once

#ifndef __GR_D3D12_COMMANDQUEUE_H_
#define __GR_D3D12_COMMANDQUEUE_H_

#include "InfluxGraphics/RHICommandQueue.h"
#include "D3D12.h"

namespace influx::Graphics
{
	class D3D12CommandList;

	/* D3D12CommandQueue */
	class D3D12CommandQueue final : public RHICommandQueue
	{
		friend class D3D12Device;
		D3D12CommandQueue(const ERHICommandQueueType type);

		struct CommandAllocatorEntry
		{
			uint64 FenceValue;
			ID3D12CommandAllocator* Allocator;
		};

		using FenceEventHandle_t = void*;

		ID3D12CommandQueue* mp_dxCommandQueue;
		ID3D12Fence* mp_dxFence;
		std::queue<CommandAllocatorEntry> m_commandAllocatorQueue;
		std::queue<D3D12CommandList*> m_commandListQueue;
		FenceEventHandle_t m_fenceEventHandle;
		uint64 m_currentFenceValue;

	public:
		virtual RHICommandList* SetupNewCommandList(RHIDevice* device) override final;
		virtual void ExecuteCommmandList(RHICommandList* commandList) override final;
		virtual void Flush() override final;

		virtual ~D3D12CommandQueue();

		ID3D12CommandQueue* GetDxCommandQueue() const;

	private:
		bool IsFenceComplete(UINT64 completeValue) const;
	};
}

#endif
