#pragma once

#pragma comment(lib, "InfluxAssets.lib")

#define INFLUX_ASSETS_USE_CORE 1

// Types...
#include "InfluxAssets/Scene/Scene.h"
#include "InfluxAssets/Shader/Shader.h"
#include "InfluxAssets/Image/Image.h"

/* influx Assets API */
namespace influx::Assets
{
	using SceneCachePtr = SceneCache*;
	using ShaderCachePtr = ShaderCache*;
	using ImageCachePtr = ImageCache*;

	/* Loads an 3D-model scene file (.fbx, .obj) */
	bool LoadSceneFile(const string& filepath, Scene& out_scene, SceneCachePtr = nullptr, const SceneLoadDesc& loadDesc = {});

	/* Loads a Shader file (.hlsl) */
	bool LoadShaderFile(const string& filepath, ShaderData& out_shaderData, ShaderCachePtr = nullptr, const ShaderLoadDesc& loadDesc = {});

	/* Loads an 2D-image (.png, .jpeg) */
	bool LoadImageFile(const string& filepath, Image& out_image, ImageCachePtr = nullptr, const ImageLoadDesc& loadDesc = {});
}