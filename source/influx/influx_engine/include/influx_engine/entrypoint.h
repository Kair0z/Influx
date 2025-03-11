#pragma once
#ifndef INFLUX_ENGINE_MAIN
#define INFLUX_ENGINE_MAIN

extern "C" { __declspec(dllexport) extern const uint32_t D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

int main()
{
	influx::engine::run_editor();
	return 0;
}

#endif