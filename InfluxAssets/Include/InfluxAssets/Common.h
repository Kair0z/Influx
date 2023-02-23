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

	struct Scene final
	{
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
	};

	struct SceneLoadDesc final
	{
		bool bNoMeshesIsValid = true;
	};

	struct Texture final
	{

	};
}