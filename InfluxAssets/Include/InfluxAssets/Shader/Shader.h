#pragma once

#if INFLUX_ASSETS_USE_CORE
#include "Core/Cache.h"
#include "Core/Container/Map.h"
#endif

namespace Influx::Assets
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

	using ShaderCache = Influx::Cache<ShaderData, String, ShaderLoadDesc>;
}

// Define SceneLoadDesc Hash function...
namespace std
{
	template <>
	struct std::hash<Influx::Assets::ShaderLoadDesc>
	{
		size_t operator()(const Influx::Assets::ShaderLoadDesc& key) const noexcept
		{
			return 0u;
		}
	};
}