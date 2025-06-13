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
#include "core/result.h"
#include "core/material/material.h"
#include "core/math/bounds.h"
#include "core/geometry/sphere.h"
#include "core/container/array.h"

// influx::shader
#include "influx_shader.h"

// influx::imp
#include "file/influx_file.h"

namespace influx::imp
{
	template <typename _t = char>
	using result = influx::result<_t, const char*>;

	/* Loads an 3D-model scene file (.fbx, .obj) */
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
			influx::camera m_camera = {};
			math::matrix4x4f m_world_transform{};
		};

		const mesh& get_main_mesh() const { return m_meshes[0]; }
		const mesh& get_mesh(const uint32 i) const { return m_meshes[i % m_meshes.size()]; }
		mesh& get_mesh(const uint32 i) { return m_meshes[i % m_meshes.size()]; }
		const vector<mesh>& get_meshes() const { return m_meshes; }
		const uint32 get_num_meshes() const { return static_cast<uint32>(m_meshes.size()); }

		vector<mesh> m_meshes{};
		vector<light> m_lights{};
		vector<camera> m_cameras{};
		vector<influx::material> m_materials{};
		uint32 m_num_materials{};
	};

	using mesh_data = scene_data::mesh;

	INFLUX_ASSETS_API 
	result<scene_data> load_scene_file(const string& filepath, const scene_load_args& args = {});

	/* Loads a Shader file (.hlsl) */
	struct shader_load_args final
	{
		shader::compile_args m_compile_args{};
	};

	struct shader_data final
	{
		shader::shader_signature m_signature;
		shader::compile_output m_compile_result;
		shader::e_shader_type m_type;
	};
	
	INFLUX_ASSETS_API /* loads a single shader in a given file */
	result<shader_data> load_shader_file(const string& filepath, const shader_load_args& load_args = {});

	INFLUX_ASSETS_API /* loads all shaders in a given file */
	result<vector<shader_data>> load_shaders_in_file(const string& filepath, const shader_load_args& load_args = {});


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

	INFLUX_ASSETS_API
	result<image_data> load_image_file(const string& filepath, const image_load_args& args = {});

	/* Loads a 3D-image (cubemap) */
	struct cubemap_load_args final
	{
		stat_array<string, 6u>* m_hacky_paths = nullptr;
	};

	struct cubemap_data final
	{
		vector<pixel32> m_pixels{};
		math::vectoru3 m_dimensions{};
	};

	INFLUX_ASSETS_API 
	result<cubemap_data> load_cubemap(const string& filepath, const cubemap_load_args& args = {});
}