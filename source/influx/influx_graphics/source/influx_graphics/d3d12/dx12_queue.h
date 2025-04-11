#pragma once
#include "influx_graphics/queue.h"
#include "influx_graphics/d3d12/dx12_base.h"

struct ID3D12CommandQueue;

namespace influx::graphics
{
	class dx12_queue final : public queue
	{
	private:
		ID3D12CommandQueue* mpdx_queue;

	private:
		dx12_queue(const queue_desc& desc, ID3D12CommandQueue* queue);
		~dx12_queue();

		virtual result<> submit_commandlists(const vector<commandlist*>& commandlists) override;

		virtual result<> queue_signal(fence* fence, uint64 value) override;

		virtual void release_impl(device*) override;
		
		friend class dx12_device;
	};
}