#include "graphics_pch.h"
#include "influx_graphics/d3d12/dx12_fence.h"
#include "influx_graphics/d3d12/dx12_queue.h"
#include "dx12_headers.h"

namespace influx::graphics
{
	dx12_fence::dx12_fence(ID3D12Fence* fence)
	{
		mp_native = mpdx_fence = fence;
		set_releasable(mpdx_fence);
	}

	// queues a signal command to the command queue
	void dx12_fence::queue_signal(uint64 value, queue* queue)
	{
		ID3D12CommandQueue* native_queue = queue->get_native<ID3D12CommandQueue>();
		native_queue->Signal(mpdx_fence, value);
	}

	void dx12_fence::signal(uint64 value)
	{
		mpdx_fence->Signal(value);
	}

	void dx12_fence::wait_for_value(uint64 value, wait_handle& handle)
	{
		while (query_value() != value)
		{
			::HANDLE event_handle = ::CreateEventEx(NULL, 0, 0, EVENT_ALL_ACCESS);

			// Fire event when GPU hits current fence.
			mpdx_fence->SetEventOnCompletion(value, event_handle);

			// Wait until the GPU hits current fence event is fired.
			::WaitForSingleObject(event_handle, static_cast<::DWORD>(handle.get_ms_max()));
			::CloseHandle(event_handle);
		}
	}

	uint64 dx12_fence::query_value() const
	{
		return mpdx_fence->GetCompletedValue();
	}
}