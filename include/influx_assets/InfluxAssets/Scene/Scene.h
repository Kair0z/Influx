#pragma once

#if INFLUX_ASSETS_USE_CORE
#include "Core/Container/Vector.h"
#include "Core/Math/Vector.h"
#include "Core/Cache.h"
#include "Core/Scene/Scene.h" // influx::Scene::ELightType
#endif

namespace influx::Assets
{
	struct SceneLoadDesc final
	{
		int temp_id = 0;

		bool operator==(const SceneLoadDesc& scene) const
		{
			// Todo...
			return true;
		}
	};

	struct Scene final
	{
		vector<influx::Scene::Mesh> Meshes{};
		vector<influx::Scene::Light> Lights{};
		vector<influx::Scene::Camera> Cameras{};

		bool operator==(const Scene& scene) const
		{
			// Todo...
			return true;
		}

		const uint64 GetTotalNumMeshes() const
		{
			return Meshes.dimension();
		}

		const uint64 GetTotalNumVertices() const
		{
			uint64 result{};

			for (uint64 m = 0u; m < Meshes.dimension(); ++m)
			{
				result += Meshes[m].GetVertices().dimension();
			}
			
			return result;
		}

		const uint64 GetVertexDataSizeInBytes() const
		{
			return GetTotalNumVertices() * sizeof(influx::Scene::Mesh::Vertex);
		}

		const uint64 GetIndexDataSizeInBytes() const
		{
			uint64 result{};

			for (uint64 m = 0; m < Meshes.dimension(); ++m)
			{
				for (uint64 i = 0; i < Meshes[m].GetIndices().dimension(); ++i)
				{
					result += sizeof(Meshes[m].GetIndices()[i]);
				}
			}

			return result;
		}
	};

	using SceneCache = influx::Cache<Scene, string, SceneLoadDesc>;
}

// Define SceneLoadDesc Hash function...
namespace std
{
	template <>
	struct std::hash<influx::Assets::SceneLoadDesc>
	{
		size_t operator()(const influx::Assets::SceneLoadDesc& key) const noexcept
		{
			return 0u;
		}
	};
}

