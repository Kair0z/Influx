#pragma once

#include "assimp/Importer.hpp"	// C++ importer interface
#include "assimp/scene.h"		// Output data structure
#include "assimp/postprocess.h"	// Post processing flags

#include "Core/Math/Vector.h"
#include "Core/String.h"
#include "Core/Scene/Light.h"
#include "Core/Scene/Camera.h"
#include "Core/Function.h"

#include "Core/Container/Map.h"


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

	inline aiString to_assimp(const string& string)
	{
		return aiString(string);
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

	inline void traverse_graph(const aiNode* root, const function<void(const aiNode*, const aiMatrix4x4&)>& func)
	{
		// depth-first traverse the node
		umap<const aiNode*, bool> visited = {};
		function<void(const aiNode*, const aiMatrix4x4&)> depth_first_traversal{};
		depth_first_traversal = [&visited, &depth_first_traversal, func](const aiNode* node, const aiMatrix4x4& parent_world_transform)
		{
			if (visited[node] == true) return;

			const aiMatrix4x4& this_world_transform = parent_world_transform * node->mTransformation;
			func(node, this_world_transform);
			
			// check children
			for (uint32 i = 0u; i < node->mNumChildren; ++i)
			{
				depth_first_traversal(node->mChildren[i], this_world_transform);
			}
		};

		depth_first_traversal(root, root->mTransformation);
	}

	struct add_mesh_info final
	{
		aiMatrix4x4 m_world_transform; 

		aiMatrix4x4 m_world_rotation;
		aiMatrix4x4 m_world_translation;
		aiMatrix4x4 m_world_scale;
		aiVector3D m_world_rotation_v;
		aiVector3D m_world_translation_v;
		aiVector3D m_world_scale_v;

		aiMaterial* m_material;

		uint32 m_idx;
	};

	inline void for_each_mesh_in(const string& filepath, const function<void(const aiMesh*, const add_mesh_info&)>& func)
	{
		const aiScene* scene = scene_from_file(filepath);
		if (scene == nullptr)
		{
			return;
		}

		traverse_graph(scene->mRootNode, [func, scene](const aiNode* node, const aiMatrix4x4& world_transform)
		{
			aiMatrix4x4 this_world_transform = world_transform * node->mTransformation;

			aiVector3D scale = {};
			aiVector3D translation = {};
			aiVector3D rotation = {};
			this_world_transform.Decompose(scale, rotation, translation);

			for (uint32 i = 0u; i < node->mNumMeshes; ++i)
			{
				const uint32 mesh_index = node->mMeshes[i];
				const aiMesh* mesh = scene->mMeshes[mesh_index];

				add_mesh_info info{};
				info.m_idx = mesh_index;
				info.m_world_transform = this_world_transform;
				info.m_world_scale_v = scale;
				info.m_world_rotation_v = rotation;
				info.m_world_translation_v = translation;
				info.m_world_scale = aiMatrix4x4::Scaling(scale, info.m_world_scale);
				info.m_world_rotation.FromEulerAnglesXYZ(rotation.x, rotation.y, rotation.z);
				info.m_world_translation = aiMatrix4x4::Translation(translation, info.m_world_translation);
				info.m_material = scene->mMaterials[mesh->mMaterialIndex];
				func(mesh, info);
			}
		});
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

	inline void for_each_texture_in(const string& filepath, const function<void(const aiTexture*, const uint32)>& func)
	{
		const aiScene* scene = scene_from_file(filepath);
		if (scene == nullptr)
		{
			return;
		}

		for (uint32 i = 0u; i < scene->mNumTextures; ++i)
		{
			func(scene->mTextures[i], i);
		}
	}

	enum class e_material_property : uint8
	{
		diffuse,
		emissive,
		ambient,
		specular,
		max
	};

	namespace detail
	{
		const static char* g_material_property_strings[static_cast<uint8>(e_material_property::max)]
		{
			"$clr.diffuse",
			"$clr.emissive",
			"$clr.ambient",
			"$clr.specular"
		};

		const static char* get_material_property_string(e_material_property prop)
		{
			return g_material_property_strings[static_cast<uint8>(prop)];
		}
	}
	
	template <typename _t>
	inline _t parse_material_property(const string& property_name, const aiMaterial* material)
	{
		_t result = {};

		for (uint32 i = 0u; i < material->mNumProperties; ++i)
		{
			const aiMaterialProperty* prop = material->mProperties[i];
			if (prop->mKey == to_assimp(property_name))
			{
				if (prop->mDataLength > sizeof(_t)) return {};

				memcpy(reinterpret_cast<void*>(&result), reinterpret_cast<void*>(prop->mData), prop->mDataLength);
			}
		}

		return result;
	}

	template <typename _t>
	inline _t parse_material_property(e_material_property prop, const aiMaterial* material)
	{
		return parse_material_property<_t>(detail::get_material_property_string(prop), material);
	}
}