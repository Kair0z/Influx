#include "common.h"
#include "core/math/matrix.h"

namespace influx::assets
{
	template<typename _t>
	class asset_data;

	enum class e_asset_type
	{
		mesh,
		scene,
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
	};

	struct actor_data final
	{

	};
}