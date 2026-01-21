#include "engine_pch.h"
#include "asset_manager.h"

// influx::assets
#include "influx_assets.h"

namespace influx::engine
{
	asset_manager::asset_manager()
	{
		assets::asset_handle asset = assets::load_flx("").get();
	}

	asset_manager::~asset_manager()
	{

	}
}