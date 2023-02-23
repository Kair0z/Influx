#include "InfluxAssets/Cache/Cache.h"

#include "InfluxAssets/InfluxAssets.h"

namespace Influx::Assets
{
	CachePtr CreateCache()
	{
		return new Cache();
	}
	
	void DestroyCache(CachePtr& pCache)
	{
		if (pCache != nullptr)
		{
			delete pCache;
			pCache = nullptr;
		}
	}

	Cache::SceneMap& Cache::GetLoadedSceneMap()
	{
		return m_loadedScenes;
	}

	Cache::MeshMap& Cache::GetLoadedMeshMap()
	{
		return m_loadedMeshes;
	}

	Cache::SceneCameraMap& Cache::GetLoadedSceneCameraMap()
	{
		return m_loadedSceneCameras;
	}

	Cache::SceneLightMap& Cache::GetLoadedSceneLightMap()
	{
		return m_loadedSceneLights;
	}
}

