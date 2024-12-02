#pragma once
#ifndef INFLUX_ENGINE_MAIN
#define INFLUX_ENGINE_MAIN

// influx::core
#include "core/basetypes.h"

// influx::engine
influx::engine::base_module* create_module();

extern "C" { __declspec(dllexport) extern const uint32_t D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

int main()
{
	influx::engine::detail::run_engine(create_module());
	return 0;
}

#endif