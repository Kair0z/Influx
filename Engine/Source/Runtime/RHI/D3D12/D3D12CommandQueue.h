#pragma once

#include "Runtime/RHI/CommandQueue.h"
#include "D3D12API.h"

#include "Core/Container/Containers.h"

namespace Influx
{
	class D3D12GraphicsCommandList;

	/* Encapsulates ID3D12CommandQueue Interface */
	class D3D12CommandQueue final : public RHICommandQueue
	{
	public:
		D3D12CommandQueue(const Ptr<D3D12API> api, D3D12_COMMAND_LIST_TYPE cmdType);
		~D3D12CommandQueue();

	public:
		/* Creates a new ready-to-populate command list, or uses one that is available */
		virtual RHIGraphicsCommandList* GetNewGraphicsCommandList(Ptr<RenderAPI> api) override final;

		/* Execute Commandlist, this does not block the calling thread...*/
		virtual void ExecuteCommandList(Ptr<RHIGraphicsCommandList> list) override final;

		/* Flush the Command Queue */
		void Flush() override final;

		uint64_t Signal();
		bool IsFenceComplete(uint64_t fenceValue);
		void WaitForFence(uint64_t f);

		ID3D12CommandQueue* GetD3D12CommandQueue() const;

	protected:
		ID3D12CommandAllocator* CreateCmdAllocator(const Ptr<D3D12API> api);
		D3D12GraphicsCommandList* CreateCommandList(const Ptr<D3D12API> api, ID3D12CommandAllocator* allocator);

	private:
		struct TCommandAllocator
		{
			uint64_t fValue;
			ID3D12CommandAllocator* alloc;
		};
		using CommandAllocatorQueue = Queue<TCommandAllocator>;
		using CommandListQueue = Queue<D3D12GraphicsCommandList*>;

		D3D12_COMMAND_LIST_TYPE mCommandType;
		ID3D12Device2* mpDeviceReference;
		ID3D12CommandQueue* mpD3D12CmdQueue;
		ID3D12Fence* mpFence;

		HANDLE mFenceEvent;
		uint64_t mFenceValue;

		CommandAllocatorQueue mCommandAllocatorQueue;
		CommandListQueue mCommandListQueue;
	};
}

