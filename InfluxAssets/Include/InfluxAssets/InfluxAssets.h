#pragma once

#include "InfluxAssets/Common.h"
#include "InfluxAssets/Scene/Scene.h"
#include "InfluxAssets/Shader/Shader.h"

/* Influx Assets API */
namespace Influx::Assets
{
	using SceneCachePtr = SceneCache*;
	using ShaderCachePtr = ShaderCache*;

	/* Loads an 3D-model scene file. */
	bool LoadScene(const String& filepath, Scene& out_scene, SceneCachePtr = nullptr, const SceneLoadDesc& loadDesc = {});

	/* Loads a Shader file (.hlsl) */
	bool LoadShader(const String& filepath, Shader& out_shader, ShaderCachePtr = nullptr, const ShaderLoadDesc& loadDesc = {});
}