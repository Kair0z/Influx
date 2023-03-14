
#include "InfluxAssets/InfluxAssets.h"

namespace Influx::Assets
{
	/* Loads a Shader file (.hlsl) */
	bool LoadShader(const String& filepath, Shader& out_shader, ShaderCachePtr pCache, const ShaderLoadDesc& loadDesc)
	{
		// Try to find the loaded scene in the provided cache...
		if (pCache)
		{
			if (pCache->Contains(filepath, loadDesc))
			{
				// Copy!
				out_shader = *pCache->Get(filepath, loadDesc);
				return true;
			}
		}

		// Create new data...
		Shader shaderData;

		// Cache loaded scene result:
		if (pCache)
		{
			if (!pCache->Contains(filepath, loadDesc))
			{
				// Copy sceneData into scene-cache!
				pCache->Add(filepath, shaderData, loadDesc);
			}
		}

		out_shader = shaderData;
		return true;
	}
}