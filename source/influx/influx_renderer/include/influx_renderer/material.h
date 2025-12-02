#pragma once
#include "common.h"

namespace influx::renderer
{
	enum class e_material : uint8
	{
		none,
		num
	};
	static constexpr uint8 k_num_internal_materials = static_cast<uint32>(e_material::num);
	static const char* k_internal_material_names[k_num_internal_materials] =
	{
		"none"
	};

	inline constexpr const char* get_internal_material_name(const e_material& mat)
	{
		return k_internal_material_names[static_cast<uint32>(mat)];
	}

	static mat_id get_internal_material_id(const e_material& mat)
	{
		return static_cast<uint32>(mat);
	}

	static bool is_internal_material(const mat_id id)
	{
		return static_cast<uint32>(id) < k_num_internal_materials;
	}
}