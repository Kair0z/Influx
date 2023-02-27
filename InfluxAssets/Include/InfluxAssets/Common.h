#pragma once

#if _DEBUG
#pragma comment(lib, "assimp-vc142-mtd.lib")
#else
#pragma comment(lib, "assimp-vc142-mt.lib")
#endif

#include "Core/BasicTypes.h"
#include "Core/String.h"
#include "Core/Container/Containers.h"
#include "Core/Math/Vector.h"
#include "Core/Scene/Scene.h"

namespace Influx::Assets
{
	enum class EAssetType
	{
		MeshScene,
		Mesh,
		Texture,
		Max
	};

	struct Texture final
	{

	};
}