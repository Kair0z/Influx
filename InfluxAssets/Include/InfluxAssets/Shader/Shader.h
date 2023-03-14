#pragma once

#include "Core/Cache.h"
#include "Core/Container/Map.h"

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

	struct Shader final
	{

		bool operator==(const Shader& scene) const
		{
			// Todo...
			return true;
		}
	};

	using ShaderCache = Influx::Cache<Shader, String, ShaderLoadDesc>;
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