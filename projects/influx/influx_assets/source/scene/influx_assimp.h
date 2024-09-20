#pragma once
#include "core/math/vector.h"
#include "core/scene/light.h"
#include "core/scene/mesh.h"

namespace influx
{
	math::vectorf2 translate(const aiVector2D& vector)
	{
		return { vector.x, vector.y };
	}

	math::vectorf3 translate(const aiVector3D& vector)
	{
		return { vector.x, vector.y, vector.z };
	}

	math::vectorf3 translate(const aiColor3D& vector)
	{
		return { vector.r, vector.g, vector.b };
	}

	math::vectorf4 translate(const aiColor4D& vector)
	{
		return { vector.r, vector.g, vector.b, vector.a };
	}

	string translate(const aiString& string)
	{
		return { string.C_Str() };
	}

	constexpr influx::scene::e_light_type translate(const aiLightSourceType& lightType)
	{
		switch (lightType)
		{
		case aiLightSource_DIRECTIONAL: return influx::scene::e_light_type::directional;
		case aiLightSource_POINT:		return influx::scene::e_light_type::point;
		case aiLightSource_SPOT:		return influx::scene::e_light_type::spot;
		default:
		case aiLightSource_UNDEFINED: return influx::scene::e_light_type::count;
		}
	}

	influx::assets::scene_data::mesh translate(const aiMesh* pMesh)
	{
		influx::assets::scene_data::mesh result{};
		constexpr uint32 vColChannel = 0u;

		const bool meshHasPositions = pMesh->HasPositions();
		const bool meshHasNormals = pMesh->HasNormals();
		const bool meshHasVertexColors = pMesh->HasVertexColors(vColChannel);

		bool collectPositions = true && meshHasPositions;
		bool collectNormals = true && meshHasNormals;
		bool collectVertexColors = true && meshHasVertexColors;
		bool collectUvs = true && pMesh->HasTextureCoords(0u);

		for (uint32 v = 0u; v < pMesh->mNumVertices; ++v)
		{
			if (collectPositions)	 result.m_positions.push_back(translate(pMesh->mVertices[v]));
			if (collectNormals)		 result.m_normals.push_back(translate(pMesh->mNormals[v]));
			if (collectVertexColors) result.m_colours.push_back(translate(pMesh->mColors[v][vColChannel]));
			if (collectUvs)			result.m_uvs.push_back(translate(pMesh->mTextureCoords[0u][v]));
		}

		for (uint32 f = 0u; f < pMesh->mNumFaces; ++f)
		{
			for (uint32 i = 0u; i < pMesh->mFaces[f].mNumIndices; ++i)
			{
				uint32 index = pMesh->mFaces[f].mIndices[i];
				result.m_indices.push_back(index);
			}
		}

		return result;
	}

	influx::scene::camera translate(const aiCamera* pCamera)
	{
		influx::scene::camera result{};

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

	influx::scene::light translate(const aiLight* pLight)
	{
		influx::scene::light result{};

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

	void* translate(const aiMaterial* material)
	{
		return nullptr;
	}

	influx::assets::scene_data parse(const aiScene* pScene)
	{
		assets::scene_data result{};

		for (uint32 i = 0u; i < pScene->mNumMeshes; ++i)
		{
			const aiMesh* mesh = pScene->mMeshes[i];
			result.m_meshes.push_back(translate(mesh));
		}
		for (uint32 i = 0u; i < pScene->mNumCameras; ++i)
		{
			const aiCamera* camera = pScene->mCameras[i];
			result.m_cameras.push_back(translate(camera));
		}
		for (uint32 i = 0u; i < pScene->mNumMaterials; ++i)
		{
			const aiMaterial* material = pScene->mMaterials[i];
			// result.Materials.push_back(AssimpHelpers::FromAssimp(material));
		}
		for (uint32 i = 0u; i < pScene->mNumLights; ++i)
		{
			const aiLight* light = pScene->mLights[i];
			result.m_lights.push_back(translate(light));
		}

		return result;
	}
}