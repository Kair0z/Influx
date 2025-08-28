#include "influx_engine.h"
#include <stdint.h>

// SDK 1.614.1
extern "C" { __declspec(dllexport) extern const uint32_t D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

int main(int argc, char* argv[])
{
	influx::engine::run_editor(argc, argv).get();
}