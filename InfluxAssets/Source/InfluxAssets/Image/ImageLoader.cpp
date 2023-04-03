#include "InfluxAssets/InfluxAssets.h"

namespace Influx::Assets
{
	/* Loads an 2D-image (.png, .jpeg) */
	bool LoadImageFile(const String& filepath, Image& out_image, ImageCachePtr pCache, const ImageLoadDesc& loadDesc)
	{
		// Try to find the loaded scene in the provided cache...
		if (pCache)
		{
			if (pCache->Contains(filepath, loadDesc))
			{
				// Copy!
				out_image = *pCache->Get(filepath, loadDesc);
				return true;
			}
		}

		// Create new data...
		Image imageData;

		// Cache loaded scene result:
		if (pCache)
		{
			if (!pCache->Contains(filepath, loadDesc))
			{
				// Copy sceneData into scene-cache!
				pCache->Add(filepath, imageData, loadDesc);
			}
		}

		out_image = imageData;

		return true;
	}
}