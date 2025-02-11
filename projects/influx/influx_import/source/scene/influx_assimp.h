#pragma once
#include "core/math/vector.h"
#include "core/scene/light.h"
#include "core/scene/mesh.h"
#include "core/log.h"
#include "core/file.h"

namespace influx
{
	struct metadata_info final
	{
		uint32 m_axis_right = 0;
		uint32 m_axis_up = 1;
		uint32 m_axis_forward = 2;

		int m_right_sign = 1;
		int m_up_sign = 1;
		int m_forward_sign = 1;

		float m_scale = 1.0f;
	};

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

	math::matrix4x4f translate(const aiMatrix4x4& mat, const metadata_info& metadata_info)
	{
		int axis_flips[4u] = {
			metadata_info.m_right_sign,
			metadata_info.m_up_sign,
			metadata_info.m_forward_sign, 1u };

		uint32 axes[3u] = { metadata_info.m_axis_right, metadata_info.m_axis_up, metadata_info.m_axis_forward };

		// .. despite what assimp docs say, 
		// it's a col major matrix and needs to be transposed :)
		aiMatrix4x4 transposed = mat;
		transposed.Transpose();

		// scale + rotation
		math::matrix4x4f new_matrix{};
		for (uint32 y = 0u; y < 3u; ++y)
		{
			const float* row = transposed[y];
			for (uint32 x = 0u; x < 3u; ++x)
			{
				new_matrix[y][x] = row[axes[x]] * axis_flips[x];
			}
		}

		// translation
		uint32 transaxes[4] = { metadata_info.m_axis_up, metadata_info.m_axis_forward, metadata_info.m_axis_right };
		for (uint32 i = 0u; i < 4u; ++i)
		{
			new_matrix[3u][i] = transposed[3u][transaxes[i]] * axis_flips[i];
		}
		new_matrix[3u][3u] = 1.0f;

		// scale the axes & the translation
		for (uint32 i = 0u; i < 3u; ++i)
		{
			new_matrix[i][i] *= metadata_info.m_scale;
			new_matrix[3][i] *= metadata_info.m_scale;
		}

		return new_matrix;
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

	constexpr influx::e_texture_semantic translate(const aiTextureType& type)
	{
		switch (type)
		{
			case aiTextureType::aiTextureType_NONE: return e_texture_semantic::none;
			case aiTextureType::aiTextureType_DIFFUSE: return e_texture_semantic::diffuse;
			case aiTextureType::aiTextureType_SPECULAR: return e_texture_semantic::specular;
			case aiTextureType::aiTextureType_AMBIENT: return e_texture_semantic::ambient;
			case aiTextureType::aiTextureType_EMISSIVE: return e_texture_semantic::emissive;
			case aiTextureType::aiTextureType_HEIGHT: return e_texture_semantic::height;
			case aiTextureType::aiTextureType_NORMALS: return e_texture_semantic::normals;
			case aiTextureType::aiTextureType_SHININESS: return e_texture_semantic::roughness;
			case aiTextureType::aiTextureType_OPACITY: return e_texture_semantic::opacity;
			case aiTextureType::aiTextureType_DISPLACEMENT: return e_texture_semantic::displacement;
			case aiTextureType::aiTextureType_LIGHTMAP: return e_texture_semantic::lightmap;
			case aiTextureType::aiTextureType_REFLECTION: return e_texture_semantic::reflection;
			case aiTextureType::aiTextureType_BASE_COLOR: return e_texture_semantic::basecolor;
			case aiTextureType::aiTextureType_NORMAL_CAMERA: return e_texture_semantic::worldnormal;
			case aiTextureType::aiTextureType_METALNESS: return e_texture_semantic::metalness;
			case aiTextureType::aiTextureType_AMBIENT_OCCLUSION: return e_texture_semantic::ambientocclusion;
			case aiTextureType::aiTextureType_UNKNOWN: return e_texture_semantic::unknown;
			case aiTextureType::aiTextureType_SHEEN: return e_texture_semantic::sheen;
			case aiTextureType::aiTextureType_CLEARCOAT: return e_texture_semantic::clearcoat;
			case aiTextureType::aiTextureType_TRANSMISSION: return e_texture_semantic::transmission;
			default:
				return e_texture_semantic::count;
		}
	}

	static const aiNode* find_node(const aiScene& scene, const aiNode& parent, const aiCamera& camera)
	{
		return parent.FindNode(camera.mName);
	}
	static const aiNode* find_node_recursive(const aiScene& scene, const aiNode& parent, const aiMesh& mesh)
	{
		// find in this node...
		for (uint32 i = 0; i < parent.mNumMeshes; ++i) 
		{
			if (scene.mMeshes[parent.mMeshes[i]] == &mesh) 
			{
				return &parent; // found
			}
		}

		// not found, traverse children...
		for (uint32 i = 0; i < parent.mNumChildren; ++i) 
		{
			const aiNode* found = find_node_recursive(scene, *parent.mChildren[i], mesh);
			if (found)
			{
				return found;
			}
		}

		return nullptr;
	}

	static aiMatrix4x4 calc_world_matrix_recursive(const aiNode& node)
	{
		if (node.mParent) 
		{
			return node.mTransformation * calc_world_matrix_recursive(*node.mParent);
		}
		return node.mTransformation;
	}

	influx::imp::scene_data::mesh translate(const aiMesh& mesh)
	{
		influx::imp::scene_data::mesh result{};
		constexpr uint32 vColChannel = 0u;

		const bool meshHasPositions = mesh.HasPositions();
		const bool meshHasNormals = mesh.HasNormals();
		const bool meshHasVertexColors = mesh.HasVertexColors(vColChannel);

		bool collectPositions = true && meshHasPositions;
		bool collectNormals = true && meshHasNormals;
		bool collectVertexColors = true && meshHasVertexColors;
		bool collectUvs = true && mesh.HasTextureCoords(0u);

		math::vectorf3 max_position;
		math::vectorf3 min_position;
		math::vectorf3 sum_position;
		const uint32 num_positions = mesh.mNumVertices;

		for (uint32 v = 0u; v < mesh.mNumVertices; ++v)
		{
			if (collectPositions)
			{
				math::vectorf3 vertex = translate(mesh.mVertices[v]);
				max_position.x = math::maximum(vertex.x, max_position.x);
				max_position.y = math::maximum(vertex.y, max_position.y);
				max_position.z = math::maximum(vertex.z, max_position.z);
				min_position.x = math::minimum(vertex.x, min_position.x);
				min_position.y = math::minimum(vertex.y, min_position.y);
				min_position.z = math::minimum(vertex.z, min_position.z);
				sum_position += vertex;
				result.m_positions.push_back(vertex);
			}

			if (collectNormals)		 result.m_normals.push_back(translate(mesh.mNormals[v]));
			if (collectVertexColors) result.m_colours.push_back(translate(mesh.mColors[v][vColChannel]));
			if (collectUvs)			result.m_uvs.push_back(translate(mesh.mTextureCoords[0u][v]));
		}

		for (uint32 f = 0u; f < mesh.mNumFaces; ++f)
		{
			if (mesh.mFaces != nullptr)
			for (uint32 i = 0u; i < mesh.mFaces[f].mNumIndices; ++i)
			{
				uint32 index = mesh.mFaces[f].mIndices[i];
				result.m_indices.push_back(index);
			}
		}

		if (num_positions > 0)
		{
			result.m_average_position = sum_position / num_positions;
			math::vectorf3 local_max = max_position - result.m_average_position;
			math::vectorf3 local_min = min_position - result.m_average_position;
			result.m_bounding_box.grow_to_contain(local_max);
			result.m_bounding_box.grow_to_contain(local_min);
			result.m_bounding_sphere.grow_to(local_max);
			result.m_bounding_sphere.grow_to(local_min);
		}
		
		result.m_material_index = mesh.mMaterialIndex;

		return result;
	}

	influx::scene::camera translate(const aiCamera& camera)
	{
		influx::scene::camera result{};
		result.set_nearplane(camera.mClipPlaneNear);
		result.set_farplane(camera.mClipPlaneFar);
		result.set_fov(camera.mHorizontalFOV);
		result.set_is_orthographic(camera.mOrthographicWidth != 0.0f);
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

	influx::material translate(const aiMaterial& mat)
	{
		influx::material result{};

		// parse properties
		for (uint32 i = 0; i < mat.mNumProperties; ++i)
		{
			aiMaterialProperty* prop = mat.mProperties[i];
			if (prop == nullptr)
			{
				logwar("influx::imp::scene_data::parse >> nullptr material property!");
				continue;
			}

			const string& property_name = translate(prop->mKey);
			switch (prop->mType)
			{
			case aiPropertyTypeInfo::aiPTI_Buffer:
			{
				logwar("influx::imp::scene_data::parse >> unsupported material property! (buffer)");
				break;
			}
			case aiPropertyTypeInfo::aiPTI_Double:
				logwar("influx::imp::scene_data::parse >> unsupported material property (double)!");
				break;

			case aiPropertyTypeInfo::aiPTI_Float:
				result.add_scalar(property_name, { static_cast<float>(*prop->mData) });
				break;

			case aiPropertyTypeInfo::aiPTI_Integer:
				result.add_int(property_name, { static_cast<int>(*prop->mData) });
				break;

			case aiPropertyTypeInfo::aiPTI_String: break;
				logwar("influx::imp::scene_data::parse >> unsupported material property (string)!");
				break;
			}
		}

		// parse textures
		for (uint32 t = 0; t < AI_TEXTURE_TYPE_MAX; ++t)
		{
			const aiTextureType type = static_cast<aiTextureType>(t);
			for (uint32 i = 0; i < mat.GetTextureCount(type); ++i)
			{
				aiString* out_path = nullptr;
				mat.GetTexture(type, i, out_path);
				if (out_path == nullptr)
				{
					logwar("influx::imp::scene_data::parse >> failed parse texture path!");
					continue;
				}

				string path = translate(*out_path);
				if (!file::exists(path))
				{
					logwar("influx::imp::scene_data::parse >> failed to find texture path!");
					continue;
				}

				file to_file = file(path);
				const auto semantic = translate(type);
				result.add_texture(semantic,
				{
					.m_semantic = semantic,
					.m_texture_index = i,
					.m_path = to_file.m_path_full
				});
			}
		}

		return result;
	}

	influx::scene::light translate(const aiLight& light)
	{
		// result.Position = FromAssimp(pLight->mPosition); // position
		// result.Direction = FromAssimp(pLight->mDirection); // forward
		// result.Name = FromAssimp(pLight->mName);
		// result.Up = FromAssimp(pLight->mUp);
		// result.AreaSize = FromAssimp(pLight->mSize);

		influx::scene::light result{};
		result.set_type(translate(light.mType));
		result.set_attenuation(light.mAttenuationConstant);
		// result.AttenuationLinear = pLight->mAttenuationLinear;
		// result.AttenuationQuadratic = pLight->mAttenuationQuadratic;
		
		result.set_colour(translate(light.mColorDiffuse));
		// result.ColorSpecular = FromAssimp(pLight->mColorSpecular);
		// result.ColorAmbient = FromAssimp(pLight->mColorAmbient);
		
		result.set_inner_angle(light.mAngleInnerCone);
		result.set_outer_angle(light.mAngleOuterCone);
		
		return result;
	}

	metadata_info parse_metadata(imp::scene_data& out_result, const aiMetadata& data)
	{
		metadata_info out_info{};

		umap<string, int> int_props{};
		umap<string, float> fl_props{};
		for (uint32 i = 0u; i < data.mNumProperties; ++i)
		{
			const aiString* out_string = nullptr;
			const aiMetadataEntry* out_entry = nullptr;
			if (data.Get(i, out_string, out_entry))
			{
				switch (out_entry->mType)
				{
				case aiMetadataType::AI_INT32: 
					memcpy(&int_props[out_string->C_Str()], out_entry->mData, sizeof(int));
					break;
				case aiMetadataType::AI_FLOAT:
					memcpy(&fl_props[out_string->C_Str()], out_entry->mData, sizeof(float));
					break;
				}
				printf(out_string->C_Str());
				printf("\n");
			}
		}

		if (int_props.contains("UpAxis"))
			out_info.m_axis_up = int_props["UpAxis"];
		if (int_props.contains("FrontAxis"))
			out_info.m_axis_forward = int_props["FrontAxis"];
		if (int_props.contains("CoordAxis"))
			out_info.m_axis_right = int_props["CoordAxis"];

		if (int_props.contains("UpAxisSign"))
			out_info.m_up_sign = int_props["UpAxisSign"];
		if (int_props.contains("FrontAxisSign"))
			out_info.m_forward_sign = int_props["FrontAxisSign"];
		if (int_props.contains("CoordAxisSign"))
			out_info.m_right_sign = int_props["CoordAxisSign"];

		printf("lol");
		return out_info;
	}

	inline influx::imp::scene_data parse(const aiScene* pScene, const imp::scene_load_args& load_args)
	{
		imp::scene_data result{};

		metadata_info metadata_info = {};
		if (pScene->mMetaData)
		{
			metadata_info = parse_metadata(result, *pScene->mMetaData);
		}
		metadata_info.m_scale = load_args.m_pre_scale;
		
		for (uint32 i = 0u; i < pScene->mNumMeshes; ++i)
		{
			const aiMesh* mesh = pScene->mMeshes[i];
			if (mesh == nullptr)
			{ 
				logwar("influx::imp::scene_data::parse >> nullptr mesh!");
				continue;
			}
			else
			{
				// translate mesh:
				imp::scene_data::mesh mesh_data = translate(*mesh);

				// calc world matrix:
				const aiNode* mesh_node = find_node_recursive(*pScene, *pScene->mRootNode, *mesh);
				if (mesh_node)
				{
					const aiMatrix4x4& world_matrix = calc_world_matrix_recursive(*mesh_node);
					mesh_data.m_world_transform = translate(world_matrix, metadata_info);
				}
				else
				{
					mesh_data.m_world_transform = math::matrix4x4f::identity();
				}

				result.m_meshes.push_back(mesh_data);				
			}
			
		}
		for (uint32 i = 0u; i < pScene->mNumCameras; ++i)
		{
			const aiCamera* camera = pScene->mCameras[i];
			if (camera == nullptr)
			{
				logwar("influx::imp::scene_data::parse >> nullptr camera!");
				continue;
			}

			imp::scene_data::camera camera_data{};
			camera_data.m_camera = translate(*camera);
			const aiNode* cam_node = find_node(*pScene, *pScene->mRootNode, *camera);
			if (cam_node)
			{
				const aiMatrix4x4& world_matrix = calc_world_matrix_recursive(*cam_node);
				camera_data.m_world_transform = translate(world_matrix, metadata_info);
			}
			else
			{
				camera_data.m_world_transform = math::matrix4x4f::identity();
			}
			result.m_cameras.push_back(camera_data);
		}
#if 0
		for (uint32 i = 0u; i < pScene->mNumMaterials; ++i)
		{
			const aiMaterial* material = pScene->mMaterials[i];
			if (material == nullptr)
			{
				logwar("influx::imp::scene_data::parse >> nullptr material!");
				continue;
			}
			result.m_materials.push_back(translate(*material));
		}
#endif
		result.m_num_materials = pScene->mNumMaterials;

		for (uint32 i = 0u; i < pScene->mNumLights; ++i)
		{
			const aiLight* light = pScene->mLights[i];
			if (light == nullptr)
			{
				logwar("influx::imp::scene_data::parse >> nullptr light!");
				continue;
			}
			result.m_lights.push_back(translate(*light));
		}

		return result;
	}
}