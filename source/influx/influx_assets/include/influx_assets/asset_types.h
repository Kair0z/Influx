#include "common.h"
#include "core/math/matrix.h"

namespace influx::assets
{
	enum class e_asset_type
	{
		transform,
		mesh,
		scene,
		actor,
		num
	};
	static const char* k_asset_typenames[]
	{
		"mesh",
		"scene"
	};

	struct transform_data final
	{
		math::matrix4x4f m_matrix;

		static result<> serialize(archiver& arch, transform_data& data)
		{
			return {};
		}
		static const char* get_type_string()
		{
			return "transform";
		}
	};

	struct mesh_data final
	{
		vector<math::float3> m_positions;
		vector<uint32> m_indices;
		transform_data m_imported_transform;

		static result<> serialize(archiver& arch, mesh_data& data)
		{
			transform_data::serialize(arch, data.m_imported_transform);
			arch.serialize(data.m_positions);
			arch.serialize(data.m_indices);
			return {};
		}

		static const char* get_type_string()
		{
			return "mesh";
		}
	};

	struct scene_data final
	{
		vector<asset_handle> m_meshes;
		vector<transform_data> m_transforms;

		static result<> serialize(archiver& arch, scene_data& data)
		{
			arch.serialize(data.m_meshes);
			return {};
		}
		static const char* get_type_string()
		{
			return "scene";
		}
	};

	struct actor_data final
	{
		static result<> serialize(archiver& arch, transform_data& data)
		{
			return {};
		}
		static const char* get_type_string()
		{
			return "actor";
		}
	};
}