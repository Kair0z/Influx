#include "graphics_pch.h"
#include "influx_graphics/d3d12/dx12_queue.h"
#include "dx12_headers.h"

#include "influx_graphics/d3d12/dx12_commandlist.h"
#include "influx_graphics/d3d12/dx12_fence.h"

namespace influx::graphics
{
	dx12_queue::dx12_queue(const queue_desc& desc, ID3D12CommandQueue* queue) 
		: queue(desc)

	{
		mp_native = mpdx_queue = queue;
	}

	dx12_queue::~dx12_queue()
	{
	}

	result<> dx12_queue::submit_commandlists(const vector<commandlist*>& commandlists)
	{
		result<> res = {};
		vector<ID3D12CommandList*> dxcmdlists = {};
		for (commandlist* list : commandlists)
		{
			dxcmdlists.push_back(list->get_native<ID3D12CommandList>());
		}

		mpdx_queue->ExecuteCommandLists(
			static_cast<uint32>(commandlists.size()), dxcmdlists.data());

		return res;
	}

	// queues a signal which the fence can later wait for
	result<> dx12_queue::queue_signal(fence* fence, uint64 value)
	{
		result<> res = {};
		ID3D12Fence* dxfence = fence->get_native<ID3D12Fence>();
		auto dxres = mpdx_queue->Signal(dxfence, value);
		if (dxres != S_OK) return result<>::make_error("error: Dx12 Queue Signal failed");

		return res;
	}

	void dx12_queue::release_impl(device*)
	{
		mpdx_queue->Release();
	}
}