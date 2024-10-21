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
	bool load_scene_file(const string& filepath, scene_data& out_scene, const scene_load_args& args)
	{
		// Create an instance of the Importer class
		Assimp::Importer importer;

		// And have it read the given file with some example postprocessing
		// Usually - if speed is not the most important aspect for you - you'll
		// probably to request more postprocessing than we do in this example.
		const aiScene* aiscene = importer.ReadFile(filepath.c_str(),
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_FlipUVs |
			aiProcess_SortByPType);

		// If the import failed, report it
		if (aiscene == nullptr)
		{
			// DoTheErrorLogging(importer.GetErrorString());
			return false;
		}

		// Processing aiScene:
		out_scene = parse(aiscene);
		return true;
	}
}