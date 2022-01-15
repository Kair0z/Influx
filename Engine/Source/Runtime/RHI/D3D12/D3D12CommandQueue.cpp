#include "pch.h"
#include "D3D12CommandQueue.h"
#include "D3D12CommandList.h"

#include "Core/Type/Type.h"

namespace Influx
{
	D3D12CommandQueue::D3D12CommandQueue(const Ptr<D3D12API> api, D3D12_COMMAND_LIST_TYPE cmdType)
	{
		auto device = api->GetDevice();
		mpD3D12CmdQueue = D3D12API::CreateCommandQueue(device, cmdType);
		mCommandType = cmdType;
		mpFence = D3D12API::CreateFence(device);
	}

	Ptr<RHIGraphicsCommandList> D3D12CommandQueue::GetNewGraphicsCommandList(Ptr<RenderAPI> api)
	{
		ID3D12CommandAllocator* commandAllocator;
		D3D12GraphicsCommandList* commandList;

		Ptr<D3D12API> d3d12 = Cast<D3D12API>(api);
		if (!d3d12) return nullptr;

		/* Get a Command Allocator */
		/* An allocator can be reused as long as it's not 'in-flight' on the CmdQueue */
		if (!mCommandAllocatorQueue.empty() && IsFenceComplete(mCommandAllocatorQueue.front().fValue))
		{
			/* the last 'launched' allocator is in front and it's fence value has been reached... */
			commandAllocator = mCommandAllocatorQueue.front().alloc;
			mCommandAllocatorQueue.pop();

			commandAllocator->Reset();
		}
		else
		{
			/* No available allocator was found... */
			commandAllocator = CreateCmdAllocator(d3d12);
		}

		/* Get a Command List */
		if (!mCommandListQueue.empty())
		{
			commandList = mCommandListQueue.front();
			mCommandListQueue.pop();

			((ID3D12GraphicsCommandList*)commandList->GetD3D12CommandList())->Reset(commandAllocator, nullptr);
		}
		else
		{
			commandList = CreateCommandList(d3d12, commandAllocator);
		}

		/* This sets private data so we can query the allocator pointer later from the commandlist ;) */
		commandList->GetD3D12CommandList()->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), commandAllocator);

		return commandList;
	}

	void D3D12CommandQueue::ExecuteCommandList(Ptr<RHIGraphicsCommandList> list)
	{
		Ptr<D3D12GraphicsCommandList> cmdList = (D3D12GraphicsCommandList*)list;
		if (!cmdList) return;

		cmdList->Close();

		/* Get the private data address pointing to the assigned CommandAllocator */
		ID3D12CommandAllocator* commandAllocator;
		UINT dataSize = sizeof(commandAllocator);
		cmdList->GetD3D12CommandList()->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator);
		
		ID3D12CommandList* const ppCommandLists[] = {
			cmdList->GetD3D12CommandList()
		};

		/* Execute these CommandLists and signal*/
		mpD3D12CmdQueue->ExecuteCommandLists(1, ppCommandLists);

		/* Signal() returns the value we will be waiting for, when the GPU is finished executing this cmdlist,
			the Fence-event will trigger... */
		uint64_t fenceValue = Signal();

		mCommandAllocatorQueue.emplace(TCommandAllocator{ fenceValue, commandAllocator });
		mCommandListQueue.push(cmdList);

		/* This is a temporary pointer */
		commandAllocator->Release();
	}

	uint64_t D3D12CommandQueue::Signal()
	{
		return D3D12API::Signal(mpD3D12CmdQueue, mpFence, mFenceValue);
	}

	bool D3D12CommandQueue::IsFenceComplete(uint64_t fenceValue)
	{
		return (mpFence->GetCompletedValue() >= fenceValue);
	}

	void D3D12CommandQueue::WaitForFence(uint64_t f)
	{
		D3D12API::WaitForFenceValue(mpFence, f, mFenceEvent);
	}

	void D3D12CommandQueue::Flush()
	{
		D3D12API::Flush(mpD3D12CmdQueue, mpFence, mFenceValue, mFenceEvent);
	}

	ID3D12CommandQueue* D3D12CommandQueue::GetD3D12CommandQueue() const
	{
		return mpD3D12CmdQueue;
	}

	ID3D12CommandAllocator* D3D12CommandQueue::CreateCmdAllocator(const Ptr<D3D12API> api)
	{
		return D3D12API::CreateCommandAllocator(api->GetDevice(), mCommandType);
	}

	D3D12GraphicsCommandList* D3D12CommandQueue::CreateCommandList(const Ptr<D3D12API> api, ID3D12CommandAllocator* allocator)
	{
		return new D3D12GraphicsCommandList(D3D12API::CreateCommandList(api->GetDevice(), allocator, mCommandType));
	}

	D3D12CommandQueue::~D3D12CommandQueue()
	{
		Flush();

		while (!mCommandAllocatorQueue.empty())
		{
			mCommandAllocatorQueue.front().alloc->Release();
			mCommandAllocatorQueue.pop();
		}

		while (!mCommandListQueue.empty())
		{
			mCommandListQueue.front()->GetD3D12CommandList()->Release();
			mCommandListQueue.pop();
		}

		mpD3D12CmdQueue->Release();
		mpFence->Release();
	}
}

