#pragma once

#ifndef _D3D12_API_H_
#define _D3D12_API_H_

#include "GraphicsAPI.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "D3DCompiler.lib")

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace Influx
{
	namespace Graphics
	{
		class D3D12API final
		{
		public:
#pragma region Conversions
			
#pragma endregion

		public:

		private:
		};
	}
}

#endif


