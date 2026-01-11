#pragma once

// influx::renderer
#include "shader.h"
#include "texture.h"
#include "mesh.h"

/*
	in influx::renderer, a resource is any big piece of data the renderer needs to build API-structures around.
	for meshes this is vertex buffers & index buffers
	for textures this is texture resources
	for shaders, this is pipeline state objects

	we define resource_data as the raw input data the user provides
	we define resource_sign as the identifier as a key to user resource_data
*/

namespace influx::renderer
{
	struct mesh_buffers final
	{
		graphics::resource* m_vertexbuffer;
		graphics::resource* m_indexbuffer;
	};

	enum class e_resource_type
	{
		cubemap,
		texture2D,
		texture3D,
		shader,
		mesh,
		num
	};
	static constexpr uint32 k_num_resource_types = static_cast<uint32>(e_resource_type::num);

	// resource-data: 
	// the struct type the user inputs into the backend::load functions (raw data)
	template <e_resource_type _t>
	using resource_data = std::tuple_element_t<static_cast<uint32>(_t), std::tuple<
		cubemap_data,
		texture2D_data,
		texture3D_data,
		shader_data,
		detail::base_mesh_data const*
		>>;

	// resource-signature:
	// the unique signature struct used as the key for the map
	template <e_resource_type _t>
	using resource_sign = std::tuple_element_t<static_cast<uint32>(_t), std::tuple<
		cubemap_id,
		tex_id,
		tex_id,
		shader_id,
		mesh_id
		>>;

	// resource-type: the graphics::resource objects matching the resource type
	template <e_resource_type _t>
	using resource_type = std::tuple_element_t<static_cast<uint32>(_t), std::tuple<
		cubemap,
		texture2D,
		texture3D,
		void,
		mesh_buffers
		>>;
}