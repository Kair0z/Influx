#pragma once

// We're using Assimp libary for loading .FBX files...
#if _DEBUG
#pragma comment(lib, "assimp-vc142-mtd.lib")
#else
#pragma comment(lib, "assimp-vc142-mt.lib")
#endif

// Types...
#include "InfluxAssets/Scene/Scene.h"
#include "InfluxAssets/Shader/Shader.h"
#include "InfluxAssets/Image/Image.h"

/* Influx Assets API */
namespace Influx::Assets
{
	using SceneCachePtr = SceneCache*;
	using ShaderCachePtr = ShaderCache*;
	using ImageCachePtr = ImageCache*;

	/* Loads an 3D-model scene file (.fbx, .obj) */
	bool LoadScene(const String& filepath, Scene& out_scene, SceneCachePtr = nullptr, const SceneLoadDesc& loadDesc = {});

	/* Loads a Shader file (.hlsl) */
	bool LoadShader(const String& filepath, Shader& out_shader, ShaderCachePtr = nullptr, const ShaderLoadDesc& loadDesc = {});

	/* Loads an 2D-image (.png, .jpeg) */
	bool LoadImage(const String& filepath, Image& out_image, ImageCachePtr = nullptr, const ImageLoadDesc& loadDesc = {});
}