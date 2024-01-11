#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include "d3dx12.h"

namespace influx::graphics
{
	// interface base class for each object created by our graphics api
	class base
	{
	public:
		template <typename _t>
		inline _t* get_native() const
		{
			return dynamic_cast<_t*>(mp_native);
		}

		template <typename _t>
		inline _t*& get_native()
		{
			return dynamic_cast<_t*>(mp_native);
		}

	protected:
		void* mp_native;
	};
}