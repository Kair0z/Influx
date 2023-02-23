#pragma once

#include "InfluxAssets/Common.h"
#include "Core/Container/Map.h"

namespace Influx::Assets
{
	struct Scene;
	struct Mesh;
	struct Scene::Camera;
	struct Scene::Light;

	class Cache final
	{
	public:
		using SceneMap			= UMap<String, Scene>;
		using MeshMap			= UMap<String, Vector<Mesh>>;
		using SceneCameraMap	= UMap<String, Vector<Scene::Camera>>;
		using SceneLightMap		= UMap<String, Vector<Scene::Light>>;

		SceneMap& GetLoadedSceneMap();
		MeshMap& GetLoadedMeshMap();
		SceneCameraMap& GetLoadedSceneCameraMap();
		SceneLightMap& GetLoadedSceneLightMap();

	private:
		SceneMap		m_loadedScenes;
		MeshMap			m_loadedMeshes;
		SceneCameraMap	m_loadedSceneCameras;
		SceneLightMap	m_loadedSceneLights;
	};
}


