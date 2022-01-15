#include "pch.h"
#include "RenderAPI.h"

#if PLATFORM_WINDOWS
#include "D3D12/D3D12API.h"
#endif

namespace Influx
{
	Ptr<RenderAPI> RenderAPI::Create()
	{
#if PLATFORM_WINDOWS
		return new D3D12API();
#endif
	}
}

