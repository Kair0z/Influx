#pragma once

#include "InfluxAssets/Common.h"
#include "InfluxAssets/Scene/Scene.h"

/* Influx Assets API */
namespace Influx::Assets
{
	using SceneCachePtr = SceneCache*;

	/* Loads an 3D-model scene file. */
	bool LoadScene(const String& filepath, Scene& out_scene, SceneCachePtr = nullptr, const SceneLoadDesc& loadDesc = {});
}