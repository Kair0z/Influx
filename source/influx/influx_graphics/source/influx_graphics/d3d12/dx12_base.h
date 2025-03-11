#pragma once
#include "influx_graphics/base.h"

struct IUnknown;

namespace influx::graphics
{
	class dx12_base
	{
	public:
		virtual ~dx12_base();

	protected:
		void set_releasable(IUnknown* releasable);

	private:
		IUnknown* mp_releasable = nullptr;
	};
}