#include "pch.h"
#include "ResourceManager.h"

namespace Influx
{
	Ptr<ResourceManager> ResourceManager::Create()
	{
		Ptr<ResourceManager> newAssetManager = new ResourceManager();

		return newAssetManager;
	}
}