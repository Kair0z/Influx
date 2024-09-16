#pragma once
#include "influx_graphics/commandqueue.h"

struct ID3D12CommandQueue;

namespace influx::graphics
{
	class dx12_commandqueue final : public command_queue
	{
	public:
		explicit dx12_commandqueue(const command_queue_desc& desc, ID3D12CommandQueue* queue);

		virtual void submit_commandlists(const vector<commandlist*>& commandlists) override;

		virtual void queue_signal(fence* fence, uint64 value) override;

	private:
		ID3D12CommandQueue* mpdx_command_queue;
	};
}