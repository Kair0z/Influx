#include "InfluxGraphics/Common.h"
#include "InfluxGraphics/D3D12/D3D12CommandList.h"
#include "InfluxGraphics/D3D12/D3D12CommandQueue.h"
#include "InfluxGraphics/D3D12/D3D12Device.h"

namespace Influx::Graphics
{
	RHICommandList* D3D12CommandQueue::SetupNewCommandList(RHIDevice* device)
	{
		ID3D12CommandAllocator* dxCmdAllocator;
		D3D12CommandList* returnList;

		D3D12Device* dxDevice = (D3D12Device*)device;

		/* Get a Command Allocator */
		/* The allocator can be reused as long as it's not 'in-flight' on the CmdQueue */
		if (!m_commandAllocatorQueue.empty() && IsFenceComplete(m_commandAllocatorQueue.front().FenceValue))
		{
			/* The last 'launched' allocator is in front and its fence value has been reached */
			dxCmdAllocator = m_commandAllocatorQueue.front().Allocator;
			m_commandAllocatorQueue.pop();

			dxCmdAllocator->Reset();
		}
		else
		{
			/* No available allocator */
			dxCmdAllocator = D3D12::CreateDxCommandAllocator(dxDevice->GetDxDevice(), Conversion::ToDx12(GetType()));
		}

		/* Get a Command List */
		if (!m_commandListQueue.empty())
		{
			/* Pop off the front list */
			returnList = m_commandListQueue.front();
			m_commandListQueue.pop();

			((ID3D12GraphicsCommandList*)returnList->GetDxCommandList())->Reset(dxCmdAllocator, nullptr);
		}
		else
		{
			returnList = new D3D12CommandList(GetType());
			returnList->mp_dxCommandList = D3D12::CreateDxCommandList(dxDevice->GetDxDevice(), dxCmdAllocator, Conversion::ToDx12(GetType()));
		}

		/* Set private data so we can query the allocator pointer later from the commandlist :) */
		returnList->GetDxCommandList()->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), dxCmdAllocator);

		return returnList;
	}

	void D3D12CommandQueue::ExecuteCommmandList(RHICommandList* commandList)
	{
		D3D12CommandList* d3d12CmdList = (D3D12CommandList*)commandList;
		ID3D12GraphicsCommandList* dxCmdList = d3d12CmdList->GetDxCommandList();
		
		if (dxCmdList == nullptr)
		{
			// ...
		}

		dxCmdList->Close();

		/* Get the private held Command Allocator */
		ID3D12CommandAllocator* cmdAllocator;
		UINT dataSize = sizeof(cmdAllocator);
		dxCmdList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &cmdAllocator);

		/* Execute */
		ID3D12CommandList* const ppCommandLists[] = { dxCmdList };
		mp_dxCommandQueue->ExecuteCommandLists(1, ppCommandLists);

		/* Signal() returns the value we will be waiting for, when the GPU is finished executing this cmdlist,
			the Fence-event will trigger... */
		UINT64 fenceValue = D3D12::Signal(mp_dxCommandQueue, mp_dxFence, m_currentFenceValue);

		/* Update queues */
		m_commandAllocatorQueue.emplace(CommandAllocatorEntry{ fenceValue, cmdAllocator });
		m_commandListQueue.push(d3d12CmdList);

		/* Release the temporary retrieved allocator pointer */
		D3D12::SafeRelease(cmdAllocator);
	}

	void D3D12CommandQueue::Flush()
	{
		D3D12::FlushCommandQueue(mp_dxCommandQueue, mp_dxFence, m_currentFenceValue, m_fenceEventHandle);
	}

	D3D12CommandQueue::~D3D12CommandQueue()
	{
		Flush();

		while (!m_commandAllocatorQueue.empty())
		{
			D3D12::SafeRelease(m_commandAllocatorQueue.front().Allocator);
			m_commandAllocatorQueue.pop();
		}

		while (!m_commandListQueue.empty())
		{
			D3D12::SafeRelease(m_commandListQueue.front()->mp_dxCommandList);
			m_commandListQueue.pop();
		}

		D3D12::SafeRelease(mp_dxCommandQueue);
		D3D12::SafeRelease(mp_dxFence);
	}

	bool D3D12CommandQueue::IsFenceComplete(UINT64 completeValue) const
	{
		return mp_dxFence->GetCompletedValue() >= completeValue;
	}

	ID3D12CommandQueue* D3D12CommandQueue::GetDxCommandQueue() const
	{
		return mp_dxCommandQueue;
	}
}