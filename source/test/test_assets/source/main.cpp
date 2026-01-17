
#include "core/string.h"
#include "influx_assets.h"
#include <iostream>

void print(const char* mssg)
{
	std::cout << mssg << "\n";
}

int main()
{
	using namespace influx::assets;
	
	asset_bundle assets = load_fbx("./scene.fbx").get();

	int idx = 0u;
	for (const asset_handle& id : assets.get_assets())
	{
		scene_data const* data		= get_data<scene_data>(id).get();
		const asset_header header	= get_header(id).get();
		const asset_meta meta		= get_meta(id).get();
		
		const influx::string filepath = "./scene_gen_" + influx::string(idx) + ".flx";
		write_flx(filepath, id, write_file_args::allow_overwrite() ).get();
		++idx;
		
		asset_handle asset = load_flx(filepath).get();
	}
}