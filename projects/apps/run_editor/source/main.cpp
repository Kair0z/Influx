#include "influx_engine.h"

#include "core/basetypes.h"

extern "C" { __declspec(dllexport) extern const uint32_t D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

int main()
{
	influx::engine::run_editor();
}
