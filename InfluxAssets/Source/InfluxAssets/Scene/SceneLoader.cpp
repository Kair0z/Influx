#include "InfluxAssets/InfluxAssets.h"

#ifdef epsilon
#undef epsilon
#endif

#include "assimp/Importer.hpp"	// C++ importer interface
#include "assimp/scene.h"		// Output data structure
#include "assimp/postprocess.h"	// Post processing flags

namespace Influx::Assets
{
	namespace AssimpHelpers
	{
		Math::Vectorf2 FromAssimp(const aiVector2D& vector)
		{
			return Math::Vectorf3{ vector.x, vector.y };
		}

		Math::Vectorf3 FromAssimp(const aiVector3D& vector)
		{
			return Math::Vectorf3{ vector.x, vector.y, vector.z };
		}

		Math::Vectorf3 FromAssimp(const aiColor3D& vector)
		{
			return Math::Vectorf3{ vector.r, vector.g, vector.b };
		}

		Math::Vectorf4 FromAssimp(const aiColor4D& vector)
		{
			return Math::Vectorf4{ vector.r, vector.g, vector.b, vector.a };
		}

		String FromAssimp(const aiString& string)
		{
			String result{ string.C_Str() };
			return result;
		}

		constexpr Influx::Scene::ELightType FromAssimp(const aiLightSourceType& lightType)
		{
			switch (lightType)
			{
			case aiLightSource_DIRECTIONAL: return Influx::Scene::ELightType::Directional;
			case aiLightSource_POINT:		return Influx::Scene::ELightType::Point;
			case aiLightSource_SPOT:		return Influx::Scene::ELightType::Spot;
			default:
			case aiLightSource_UNDEFINED:
				return Influx::Scene::ELightType::Unknown;
			}
		}

		// Todo... There's multiple colour channels!
		Influx::Scene::Mesh FromAssimp(const aiMesh* pMesh)
		{
			Influx::Scene::Mesh result{};
			constexpr uint32 vColChannel = 0u;

			const bool meshHasPositions = pMesh->HasPositions();
			const bool meshHasNormals = pMesh->HasNormals();
			const bool meshHasVertexColors = pMesh->HasVertexColors(vColChannel);

			bool collectPositions = true && meshHasPositions;
			bool collectNormals = true && meshHasNormals;
			bool collectVertexColors = true && meshHasVertexColors;

			for (uint32 v = 0u; v < pMesh->mNumVertices; ++v)
			{
				Influx::Scene::Mesh::Vertex vertex{};

				if (collectPositions)	 vertex.Position = FromAssimp(pMesh->mVertices[v]);
				if (collectNormals)		 vertex.Normal = FromAssimp(pMesh->mNormals[v]);
				if (collectVertexColors) vertex.Colour = FromAssimp(pMesh->mColors[v][vColChannel]);

				result.AddVertex(vertex);
			}

			for (uint32 f = 0u; f < pMesh->mNumFaces; ++f)
			{
				for (uint32 i = 0u; i < pMesh->mFaces[f].mNumIndices; ++i)
				{
					Influx::Scene::Mesh::Index index = pMesh->mFaces[f].mIndices[i];
					result.AddIndex(index);
				}
			}

#if CORE_SCENE_MESH_DEBUG
			result.SetName( FromAssimp(pMesh->mName) );
#endif

			return result;
		}

		Influx::Scene::Camera FromAssimp(const aiCamera* pCamera)
		{
			Influx::Scene::Camera result{};

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

		Influx::Scene::Light FromAssimp(const aiLight* pLight)
		{
			Influx::Scene::Light result{};

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

		Scene FromAssimp(const aiScene* pScene)
		{
			Scene result{};

			// Get Meshes...
			for (uint32 i = 0u; i < pScene->mNumMeshes; ++i)
			{
				const aiMesh* mesh = pScene->mMeshes[i];
				result.Meshes.push_back(AssimpHelpers::FromAssimp(mesh));
			}

			// Get Cameras...
			for (uint32 i = 0u; i < pScene->mNumCameras; ++i)
			{
				const aiCamera* camera = pScene->mCameras[i];
				result.Cameras.push_back(AssimpHelpers::FromAssimp(camera));
			}

			// Get Materials...
			for (uint32 i = 0u; i < pScene->mNumMaterials; ++i)
			{
				const aiMaterial* material = pScene->mMaterials[i];
				// result.Materials.push_back(AssimpHelpers::FromAssimp(material));
			}

			// Get Lights...
			for (uint32 i = 0u; i < pScene->mNumLights; ++i)
			{
				const aiLight* light = pScene->mLights[i];
				result.Lights.push_back(AssimpHelpers::FromAssimp(light));
			}

			return result;
		}
	}

	bool LoadScene(const String& filepath, Scene& out_scene, SceneCachePtr pCache, const SceneLoadDesc& loadDesc)
	{
		// Try to find the loaded scene in the provided cache...
		if (pCache)
		{
			if (pCache->Contains(filepath, loadDesc))
			{
				// Copy!
				out_scene = *pCache->Get(filepath, loadDesc);
				return true;
			}
		}

		// Create an instance of the Importer class
		Assimp::Importer importer;

		// And have it read the given file with some example postprocessing
		// Usually - if speed is not the most important aspect for you - you'll
		// probably to request more postprocessing than we do in this example.
		const aiScene* scene = importer.ReadFile(filepath.c_str(),
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType);

		// If the import failed, report it
		if (scene == nullptr)
		{
			// DoTheErrorLogging(importer.GetErrorString());
			return false;
		}

		// Processing aiScene:
		Scene sceneData = AssimpHelpers::FromAssimp(scene);

		// Cache loaded scene result:
		if (pCache)
		{
			if (!pCache->Contains(filepath, loadDesc))
			{
				// Copy sceneData into scene-cache!
				pCache->Add(filepath, sceneData, loadDesc);
			}
		}

		out_scene = sceneData;
		return true;
	}
}
