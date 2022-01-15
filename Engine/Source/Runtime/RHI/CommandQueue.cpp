#include "pch.h"
#include "CommandQueue.h"

#ifdef PLATFORM_WINDOWS
#include "D3D12/D3D12CommandQueue.h"
#endif

#include "Core/Type/Type.h"

namespace Influx
{
	Ptr<RHICommandQueue> RHICommandQueue::Create(const Ptr<RenderAPI> api, const CommandQueueDesc&)
	{
#ifdef PLATFORM_WINDOWS
		return new D3D12CommandQueue(Cast<D3D12API>(api), D3D12_COMMAND_LIST_TYPE_DIRECT);
#endif
		return nullptr;
	}
}

