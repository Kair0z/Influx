#pragma once

#include "InfluxAssets/Common.h"

namespace Influx::Assets
{
	class Cache;
	using CachePtr = Cache*;

	/* 
	* Create a Cache to gather the loading results into!
	* Passing it on avoids re-loading BASED ON FILEPATH the same files...
	*/
	CachePtr CreateCache();
	void DestroyCache(CachePtr& pCache);

	/* Loads the first mesh from file. */
	bool LoadMesh(const std::string& filepath, Mesh& out_ref, CachePtr pCache = nullptr, const SceneLoadDesc& loadDesc = {});

	/* Loads the mesh at [meshIndex] from file. */
	bool LoadMesh(const std::string& filepath, uint32 meshIndex, Mesh& out_ref, CachePtr pCache = nullptr, const SceneLoadDesc& loadDesc = {});
	
	/* Loads a mesh with [meshName] from file. */
	bool LoadMesh(const std::string& filepath, const String& meshName, Mesh& out_ref, CachePtr pCache = nullptr, const SceneLoadDesc& loadDesc = {});

	/* Loads all meshes from file. */
	bool LoadMeshes(const String& filepath, Vector<Mesh>& out_meshes, CachePtr pCache = nullptr, const SceneLoadDesc& loadDesc = {});

	/* Loads an 3D-model scene file. */
	bool LoadScene(const String& filepath, Scene& out_scene, CachePtr = nullptr, const SceneLoadDesc& loadDesc = {});
}