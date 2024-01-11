#pragma once
#include "influx_graphics/commandqueue.h"
#include "influx_graphics/d3d12/dx12_commandlist.h"

// dx12 includes
#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include "d3dx12.h"

// helpers
#include "influx_graphics/d3d12/dx12_helpers.h"

namespace influx::graphics
{
	class dx12_commandqueue final : public command_queue
	{
	public:
		explicit dx12_commandqueue(const command_queue_desc& desc, ID3D12CommandQueue* queue)
			: command_queue(desc)
		{
			mp_native = mpdx_command_queue = queue;
		}

		virtual void submit_commandlists(const vector<command_list*>& commandlists) override
		{
			vector<ID3D12CommandList*> dxcmdlists = {};
			for (command_list* list : commandlists)
			{
				dxcmdlists.push_back(list->get_native<ID3D12CommandList>());
			}

			mpdx_command_queue->ExecuteCommandLists(
				static_cast<uint32>(commandlists.size()), dxcmdlists.data());
		}

	private:
		ID3D12CommandQueue* mpdx_command_queue;
	};
}