#pragma once

#if _DLL
#define INFLUX_ASSETS_API __declspec(dllexport)
#else
#define INFLUX_ASSETS_API __declspec(dllimport)
#endif

// influx::core
#include "core/scene/mesh.h"
#include "core/scene/camera.h"
#include "core/scene/light.h"
#include "core/container/vector.h"
#include "core/basetypes.h"
#include "core/shader.h"
#include "core/material/material.h"
#include "core/math/bounds.h"
#include "core/geometry/sphere.h"

// influx::shader
#include "influx_shader.h"

// influx::imp
#include "file/influx_file.h"

namespace influx::imp
{
	struct scene_load_args final
	{
		float	m_pre_scale = 1.0f;

		// if true, scene_data::mesh::m_world_transform will always end up identity, and the vertex data will be pre-transformed.
		bool	m_bake_transforms = false;
	};

	struct scene_data final
	{
		struct mesh final
		{
			vector<uint32> m_indices{};
			vector<math::vectorf3> m_positions{};
			vector<math::vectorf4> m_colours{};
			vector<math::vectorf3> m_normals{};
			vector<math::vectorf2> m_uvs{};
			math::boxf m_bounding_box{};
			math::spheref m_bounding_sphere{};
			math::vectorf3 m_average_position;
			uint32 m_material_index{};

			math::matrix4x4f m_world_transform{};
		};

		struct camera final
		{
			scene::camera m_camera = {};
			math::matrix4x4f m_world_transform{};
		};

		const mesh& get_main_mesh() const { return m_meshes[0]; }
		const mesh& get_mesh(const uint32 i) const { return m_meshes[i % m_meshes.size()]; }
		mesh& get_mesh(const uint32 i) { return m_meshes[i % m_meshes.size()]; }
		const vector<mesh>& get_meshes() const { return m_meshes; }
		const uint32 get_num_meshes() const { return static_cast<uint32>(m_meshes.size()); }

		vector<mesh> m_meshes{};
		vector<scene::light> m_lights{};
		vector<camera> m_cameras{};
		vector<influx::material> m_materials{};
		uint32 m_num_materials{};
	};

	using mesh_data = scene_data::mesh;

	/* Loads an 3D-model scene file (.fbx, .obj) */
	INFLUX_ASSETS_API bool load_scene_file(const string& filepath, 
		scene_data& out_scene, const scene_load_args& args = {});


	/* Loads a Shader file (.hlsl) */
	struct shader_data final
	{
		shader::shader_signature m_signature;
		shader::compile_output m_compile_result;
		shader::e_shader_type m_type;
	};
	
	INFLUX_ASSETS_API bool load_shader_file(const string& filepath, 
		shader_data& out_shader, const shader::compile_args& args = {});


	/* Loads an 2D-image (.png, .jpeg) */
	struct image_load_args final
	{
	};

	enum class e_image_colour_type
	{
		rgb,
		rgba,
		grey,
		count
	};

	struct image_data final
	{
		vector<pixel32> m_pixels{};
		math::vectoru2 m_dimensions{};
	};

	INFLUX_ASSETS_API bool load_image_file(const string& filepath, 
		image_data& out_image, const image_load_args& args = {});
}