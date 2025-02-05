#include "renderer_pch.h"
#include "shader_manager.h"

namespace influx::renderer
{
	shader_manager::shader_manager()
		: m_shadermaps{}
	{
		for (uint8 i = 0u; i < k_num_shadermaps; ++i)
		{
			m_shadermaps[i] = shader_map(get_shadertype(i), get_shadertarget(i));
		}
	}
}