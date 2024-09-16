#include "graphics_pch.h"
#include "influx_graphics/d3d12/dx12_commandqueue.h"
#include "dx12_headers.h"

#include "influx_graphics/d3d12/dx12_commandlist.h"
#include "influx_graphics/d3d12/dx12_fence.h"

namespace influx::graphics
{
	dx12_commandqueue::dx12_commandqueue(const command_queue_desc& desc, ID3D12CommandQueue* queue)
		: command_queue(desc)
	{
		mp_native = mpdx_command_queue = queue;
	}

	void dx12_commandqueue::submit_commandlists(const vector<commandlist*>& commandlists)
	{
		vector<ID3D12CommandList*> dxcmdlists = {};
		for (commandlist* list : commandlists)
		{
			dxcmdlists.push_back(list->get_native<ID3D12CommandList>());
		}

		mpdx_command_queue->ExecuteCommandLists(
			static_cast<uint32>(commandlists.size()), dxcmdlists.data());
	}

	// queues a signal to the target fence
	void dx12_commandqueue::queue_signal(fence* fence, uint64 value)
	{
		fence->queue_signal(value, this);
	}
}