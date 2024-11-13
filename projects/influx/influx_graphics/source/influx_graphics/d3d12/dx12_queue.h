#pragma once
#include "influx_graphics/queue.h"
#include "influx_graphics/d3d12/dx12_base.h"

struct ID3D12CommandQueue;

namespace influx::graphics
{
	class dx12_queue final : public queue
	{
	public:
		explicit dx12_queue(const queue_desc& desc, ID3D12CommandQueue* queue);

		virtual void submit_commandlists(const vector<commandlist*>& commandlists) override;

		virtual void queue_signal(fence* fence, uint64 value) override;

	private:
		ID3D12CommandQueue* mpdx_queue;

		virtual void release() override;
	};
}