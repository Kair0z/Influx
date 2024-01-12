#include "graphics_pch.h"
#include "influx_graphics/d3d12/dx12_commandqueue.h"
#include "dx12_headers.h"

#include "influx_graphics/d3d12/dx12_commandlist.h"

namespace influx::graphics
{
	dx12_commandqueue::dx12_commandqueue(const command_queue_desc& desc, ID3D12CommandQueue* queue)
		: command_queue(desc)
	{
		mp_native = mpdx_command_queue = queue;
	}

	void dx12_commandqueue::submit_commandlists(const vector<command_list*>& commandlists)
	{
		vector<ID3D12CommandList*> dxcmdlists = {};
		for (command_list* list : commandlists)
		{
			dxcmdlists.push_back(list->get_native<ID3D12CommandList>());
		}

		mpdx_command_queue->ExecuteCommandLists(
			static_cast<uint32>(commandlists.size()), dxcmdlists.data());
	}
}