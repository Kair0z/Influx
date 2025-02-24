#include "renderer_pch.h"
#include "resource_manager.h"

namespace influx::renderer
{
	resource_manager::resource_manager()
	{
        // dummy texture
        {
            texture_data dummy_data{};
            dummy_data.m_width = 256u;
            for (size_t i = 0u; i < 256u * 256u; ++i)
            {
                dummy_data.m_pixels.push_back(make_pixel32(255u, 255u, 255u, 255u));
            }
            load<e_resource_type::texture>("none", dummy_data, false);
        }
        
        // dummy 
	}

	resource_manager::~resource_manager()
	{
	}
}

