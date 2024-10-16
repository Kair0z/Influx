#include "graphics_pch.h"
#include "dx12_base.h"

#include "dx12_headers.h"

namespace influx::graphics
{
	dx12_base::~dx12_base()
	{
		if (mp_releasable)
		{
			mp_releasable->Release();
		}
	}

	void dx12_base::set_releasable(IUnknown* releasable)
	{
		influx_assert(mp_releasable == nullptr);
		mp_releasable = releasable;
	}
}