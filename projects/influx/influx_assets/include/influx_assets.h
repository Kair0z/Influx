#pragma once

#if _DLL
#define INFLUX_ASSETS_API __declspec(dllexport)
#else
#define INFLUX_ASSETS_API __declspec(dllimport)
#endif

#include "core/scene/mesh.h"
#include "core/scene/camera.h"
#include "core/scene/light.h"
#include "core/container/vector.h"
#include "core/basetypes.h"
#include "core/shader.h"

namespace influx::assets
{
	/* Loads an 3D-model scene file (.fbx, .obj) */
	struct scene_load_args final
	{
		// ...
	};

	struct scene_data final
	{
		vector<scene::mesh> m_meshes{};
		vector<scene::light> m_lights{};
		vector<scene::camera> m_cameras{};
	};

	INFLUX_ASSETS_API bool load_scene_file(const string& filepath, 
		scene_data& out_scene, const scene_load_args& args = {});

	/* Loads a Shader file (.hlsl) */
	struct shader_load_args final
	{
		e_shader_type m_type;
		e_shader_target m_target;
		string m_entrypoint;
		vector<string> m_defines;

		bool m_compile_debug;
		bool m_reflection;
		bool m_pbd;
	};

	struct shader_data final
	{
		using compiled_shader = vector<byte>;
		compiled_shader m_compile_result;
	};
	
	INFLUX_ASSETS_API bool load_shader_file(const string& filepath, 
		shader_data& out_shader, const shader_load_args& args = {});


	/* Loads an 2D-image (.png, .jpeg) */
	struct image_load_args final
	{

	};

	struct image_data final
	{

	};

	INFLUX_ASSETS_API bool load_image_file(const string& filepath, 
		image_data& out_image, const image_load_args& args = {});
}