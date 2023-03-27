#pragma once

#include "Core/Container/Vector.h"
#include "Core/Math/Vector.h"
#include "Core/Cache.h"

#include "Core/Scene/Scene.h" // Influx::Scene::ELightType

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
		struct Mesh final
		{
			using Index = uint32;
			using Face = Vector<Index>;

			Mesh() = default;
			Mesh(uint32 numVertices)
			{
				Vertices.reserve(numVertices);
			}

			struct Vertex final
			{
				Math::Vectorf3 Position;
				Math::Vectorf3 Normal;
				Math::Vectorf4 Color;
			};

			Vector<Vertex> Vertices;
			Vector<Face> Faces;
			Vector<Index> Indices;

			String Name;
		};

		struct Light final
		{
			String Name;

			Influx::Scene::ELightType LightType;
			Math::Vectorf3 Position;
			Math::Vectorf3 Direction;
			Math::Vectorf3 Up;

			Math::Vectorf3 ColorDiffuse;
			Math::Vectorf3 ColorSpecular;
			Math::Vectorf3 ColorAmbient;

			float AttenuationConstant;
			float AttenuationLinear;
			float AttenuationQuadratic;

			float AngleInnerCone;
			float AngleOuterCone;

			// For Area lights (unsupported)
			Math::Vectorf2 AreaSize;
		};

		struct Camera final
		{
			String Name;

			Math::Vectorf3 Position;
			Math::Vectorf3 Up;
			Math::Vectorf3 Forward;
			float HorizontalFov;
			float NearZ;
			float FarZ;
			float AspectRatio;
			float OrthographicWidth;

			bool bIsOrthoGraphic;
		};

		struct Material final
		{

		};

		Vector<Mesh> Meshes{};
		Vector<Light> Lights{};
		Vector<Camera> Cameras{};
		Vector<Material> Materials{};

		bool operator==(const Scene& scene) const
		{
			// Todo...
			return true;
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

