
#include "influx_application.h"

extern "C" { __declspec(dllexport) extern const uint32_t D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

int main(int argc, char** argv)
{
	using namespace influx;
	application::run_args args{ argc, argv };

	args.m_assets_dir = "";
	args.m_resources_dir = "";
	args.m_commandlet = false;
	args.m_enable_editor = true;
	args.m_enable_game = true;
	args.m_enable_scenerender = true;
	args.m_name = "influx game";
	args.m_single_threaded = false;
	args.m_staged = false;
	args.m_vsync = true;
	args.m_window_clear_colour = {};
	args.m_window_width = 1280u;
	args.m_window_height = 720u;
	
	application::run(args);
}