#include "import_pch.h"
#include "influx_import.h"

#ifdef epsilon
#undef epsilon
#endif

// We're using Assimp libary for loading .FBX files...
#if INFLUX_DEBUG
#pragma comment(lib, "assimp-vc142-mtd.lib")
#else
#pragma comment(lib, "assimp-vc142-mt.lib")
#endif

#include "assimp/importer.hpp" // C++ importer interface
#include "assimp/scene.h"		// Output data structure
#include "assimp/postprocess.h"	// Post processing flags

// conversion header
#include "influx_assimp.h"

namespace influx::imp
{
	const math::matrix4x4f& scene_data::get_transform(const mesh& mesh) const
	{
		if (mesh.m_transform_index < m_world_transforms.size())
		{
			return m_world_transforms[mesh.m_transform_index];
		}

		return math::matrix4x4f::identity();
	}

	const math::matrix4x4f& scene_data::get_transform(const light& light) const
	{
		if (light.m_transform_index < m_world_transforms.size())
		{
			return m_world_transforms[light.m_transform_index];
		}

		return math::matrix4x4f::identity();
	}

	const math::matrix4x4f& scene_data::get_transform(const camera& camera) const
	{
		if (camera.m_transform_index < m_world_transforms.size())
		{
			return m_world_transforms[camera.m_transform_index];
		}

		return math::matrix4x4f::identity();
	}

	result<scene_data> load_scene_file(const string& filepath, const scene_load_args& args)
	{
		using result_type = result<scene_data>;

		// Create an instance of the Importer class
		Assimp::Importer importer;
		int step_flags =
			aiProcess_CalcTangentSpace		|
			aiProcess_Triangulate			|
			aiProcess_JoinIdenticalVertices |
			aiProcess_FlipUVs				|
			aiProcess_PopulateArmatureData  |
			aiProcess_SortByPType;

		if (args.m_bake_transforms)
		{
			step_flags |= aiProcess_PreTransformVertices;
		}
		
		const std_str filepath_std = filepath.get_std();
		const aiScene* aiscene = importer.ReadFile( filepath_std.c_str(), step_flags);
		if (aiscene == nullptr)
		{
			return result_type::make_error("Asimp::Importer::ReadFile() failed!");
		}

		scene_data out_scene{};
		out_scene = parse(aiscene, args);
		return out_scene;
	}
}