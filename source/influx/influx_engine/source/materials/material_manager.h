#pragma once

// influx::core
#include "core/container/map.h"
#include "core/material/material.h"

namespace influx::engine
{
	class material_manager final
	{
	public:
		material_manager();
		virtual ~material_manager();

		material* get_or_create_material();

	private:
		umap<string, material> m_material_map;
	};
}