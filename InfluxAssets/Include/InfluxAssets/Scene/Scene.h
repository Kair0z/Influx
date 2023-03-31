#pragma once

#if INFLUX_ASSETS_USE_CORE
#include "Core/Container/Vector.h"
#include "Core/Math/Vector.h"
#include "Core/Cache.h"
#include "Core/Scene/Scene.h" // Influx::Scene::ELightType
#endif

namespace Influx::Assets
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
		Vector<Influx::Scene::Mesh> Meshes{};
		Vector<Influx::Scene::Light> Lights{};
		Vector<Influx::Scene::Camera> Cameras{};

		bool operator==(const Scene& scene) const
		{
			// Todo...
			return true;
		}

		const uint64 GetTotalNumMeshes() const
		{
			return Meshes.size();
		}

		const uint64 GetTotalNumVertices() const
		{
			uint64 result{};

			for (uint64 m = 0u; m < Meshes.size(); ++m)
			{
				result += Meshes[m].GetVertices().size();
			}
			
			return result;
		}

		const uint64 GetVertexDataSizeInBytes() const
		{
			return GetTotalNumVertices() * sizeof(Influx::Scene::Mesh::Vertex);
		}

		const uint64 GetIndexDataSizeInBytes() const
		{
			uint64 result{};

			for (uint64 m = 0; m < Meshes.size(); ++m)
			{
				for (uint64 i = 0; i < Meshes[m].GetIndices().size(); ++i)
				{
					result += sizeof(Meshes[m].GetIndices()[i]);
				}
			}

			return result;
		}
	};

	using SceneCache = Influx::Cache<Scene, String, SceneLoadDesc>;
}

// Define SceneLoadDesc Hash function...
namespace std
{
	template <>
	struct std::hash<Influx::Assets::SceneLoadDesc>
	{
		size_t operator()(const Influx::Assets::SceneLoadDesc& key) const noexcept
		{
			return 0u;
		}
	};
}

