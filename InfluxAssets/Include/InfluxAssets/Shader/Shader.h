#pragma once

#if INFLUX_ASSETS_USE_CORE
#include "Core/Cache.h"
#include "Core/Container/Map.h"
#endif

namespace influx::Assets
{
	struct ShaderLoadDesc final
	{
		int temp_id = 0;

		bool operator==(const ShaderLoadDesc& scene) const
		{
			// Todo...
			return true;
		}
	};

	struct ShaderData final
	{
		using CompiledShader = Vector<byte>;

		CompiledShader VertexShader;
		CompiledShader PixelShader;

		bool operator==(const ShaderData& shaderData) const
		{
			// Todo...
			return true;
		}
	};

	using ShaderCache = influx::Cache<ShaderData, string, ShaderLoadDesc>;
}

// Define SceneLoadDesc Hash function...
namespace std
{
	template <>
	struct std::hash<influx::Assets::ShaderLoadDesc>
	{
		size_t operator()(const influx::Assets::ShaderLoadDesc& key) const noexcept
		{
			return 0u;
		}
	};
}