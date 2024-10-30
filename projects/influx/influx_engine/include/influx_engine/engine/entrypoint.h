#pragma once
#ifndef INFLUX_ENGINE_MAIN
#define INFLUX_ENGINE_MAIN

#include "core/basetypes.h"

extern "C" { __declspec(dllexport) extern const uint32_t D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

// defined in influx_engine
namespace influx::engine::detail
{
	void run_engine();
}

int main()
{
	influx::engine::detail::run_engine();
	return 0;
}
#endif