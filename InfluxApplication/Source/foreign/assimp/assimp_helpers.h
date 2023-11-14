#pragma once

#include "assimp/Importer.hpp"	// C++ importer interface
#include "assimp/scene.h"		// Output data structure
#include "assimp/postprocess.h"	// Post processing flags

#include "Core/Math/Vector.h"
#include "Core/String.h"
#include "Core/Scene/Light.h"
#include "Core/Scene/Camera.h"
#include "Core/Function.h"

namespace influx::assimp_helpers
{
	inline static Assimp::Importer* gp_importer = nullptr;

	inline void initialize()
	{
		gp_importer = new Assimp::Importer();
	}

	inline bool is_initialized()
	{
		return gp_importer != nullptr;
	}

	inline void ensure_initialize()
	{
		if (!is_initialized())
		{
			initialize();
		}
	}

	inline void cleanup()
	{
		delete gp_importer;
		gp_importer = nullptr;
	}

	inline math::vectorf2 from_assimp(const aiVector2D& vector)
	{
		return math::vectorf2{ vector.x, vector.y };
	}

	inline math::vectorf3 from_assimp(const aiVector3D& vector)
	{
		return math::vectorf3{ vector.x, vector.y, vector.z };
	}

	inline math::vectorf3 from_assimp(const aiColor3D& vector)
	{
		return math::vectorf3{ vector.r, vector.g, vector.b };
	}

	inline math::vectorf4 from_assimp(const aiColor4D& vector)
	{
		return math::vectorf4{ vector.r, vector.g, vector.b, vector.a };
	}

	inline string from_assimp(const aiString& string)
	{
		return { string.C_Str() };
	}

	inline constexpr scene::e_light_type from_assimp(const aiLightSourceType& lightType)
	{
		switch (lightType)
		{
		case aiLightSource_DIRECTIONAL: return influx::scene::e_light_type::directional;
		case aiLightSource_POINT:		return influx::scene::e_light_type::point;
		case aiLightSource_SPOT:		return influx::scene::e_light_type::spot;
		default:
		case aiLightSource_UNDEFINED:
			return influx::scene::e_light_type::maximum;
		}
	}

	inline scene::camera from_assimp(const aiCamera* pCamera)
	{
		influx::scene::camera result{};
		result.set_nearplane(pCamera->mClipPlaneNear);
		result.set_farplane(pCamera->mClipPlaneFar);

		// result.AspectRatio = pCamera->mAspect;
		// result.Position = FromAssimp(pCamera->mPosition);
		// result.NearZ = pCamera->mClipPlaneNear;
		// result.FarZ = pCamera->mClipPlaneFar;
		// result.Forward = FromAssimp(pCamera->mLookAt);
		// result.Up = FromAssimp(pCamera->mUp);
		// result.Name = FromAssimp(pCamera->mName);
		// result.OrthographicWidth = pCamera->mOrthographicWidth * 2.0f; // "Half horizontal orthographic width, in scene units"
		// result.HorizontalFov = pCamera->mHorizontalFOV;
		// result.bIsOrthoGraphic = pCamera->mOrthographicWidth != 0.0f;

		return result;
	}

	inline scene::light from_assimp(const aiLight* pLight)
	{
		scene::light result{};

		// result.Name = FromAssimp(pLight->mName);
		// result.LightType = FromAssimp(pLight->mType);
		// result.Position = FromAssimp(pLight->mPosition);
		// result.Direction = FromAssimp(pLight->mDirection);
		// result.Up = FromAssimp(pLight->mUp);
		// result.AttenuationConstant = pLight->mAttenuationConstant;
		// result.AttenuationLinear = pLight->mAttenuationLinear;
		// result.AttenuationQuadratic = pLight->mAttenuationQuadratic;
		// result.ColorDiffuse = FromAssimp(pLight->mColorDiffuse);
		// result.ColorSpecular = FromAssimp(pLight->mColorSpecular);
		// result.ColorAmbient = FromAssimp(pLight->mColorAmbient);
		// result.AngleInnerCone = pLight->mAngleInnerCone;
		// result.AngleOuterCone = pLight->mAngleOuterCone;
		// result.AreaSize = FromAssimp(pLight->mSize);

		return result;
	}

	inline const aiScene* scene_from_file(const string& filepath)
	{
		ensure_initialize();

		// And have it read the given file with some example postprocessing
		// Usually - if speed is not the most important aspect for you - you'll
		// probably to request more postprocessing than we do in this example.
		const aiScene* scene = gp_importer->ReadFile(filepath.c_str(),
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType);

		if (scene == nullptr)
		{
			return nullptr;
		}

		return scene;
	}

	inline const aiMesh* mesh_from_file(const string& filepath, uint8 index = 0u)
	{
		const aiScene* scene = scene_from_file(filepath);
		if (scene == nullptr)
		{
			return nullptr;
		}

		if (index >= scene->mNumMeshes)
		{
			return nullptr;
		}

		return scene->mMeshes[index];
	}

	inline void for_each_mesh_in(const string& filepath, const function<void(const aiMesh*, uint32 idx)>& func)
	{
		const aiScene* scene = scene_from_file(filepath);
		if (scene == nullptr)
		{
			return;
		}

		auto check_meshes = [func, scene](const aiNode* node)
		{
			for (uint32 i = 0u; i < node->mNumMeshes; ++i)
			{
				int idx = node->mMeshes[i];
				func(scene->mMeshes[idx], idx);
			}
		};

		aiNode* current_node = scene->mRootNode;
		while (current_node != nullptr)
		{
			check_meshes(current_node);
			// ...
		}
	}

	inline void for_each_camera_in(const string& filepath, const function<void(const aiCamera*)>& func)
	{
		const aiScene* scene = scene_from_file(filepath);
		if (scene == nullptr)
		{
			return;
		}

		for (uint32 i = 0u; i < scene->mNumCameras; ++i)
		{
			func(scene->mCameras[i]);
		}
	}

	inline void for_each_light_in(const string& filepath, const function<void(const aiLight*)>& func)
	{
		const aiScene* scene = scene_from_file(filepath);
		if (scene == nullptr)
		{
			return;
		}

		for (uint32 i = 0u; i < scene->mNumLights; ++i)
		{
			func(scene->mLights[i]);
		}
	}
}