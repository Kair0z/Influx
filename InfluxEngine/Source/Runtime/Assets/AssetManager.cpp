#include "pch.h"
#include "AssetManager.h"

namespace Influx
{
	Ptr<AssetManager> AssetManager::Create()
	{
		Ptr<AssetManager> newAssetManager = new AssetManager();

		return newAssetManager;
	}
}