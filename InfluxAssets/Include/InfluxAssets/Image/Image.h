#pragma once

#if INFLUX_ASSETS_USE_CORE
#include "Core/Cache.h"
#include "Core/Container/Map.h"
#endif

namespace influx::Assets
{
	struct ImageLoadDesc final
	{
		int temp_id = 0;

		bool operator==(const ImageLoadDesc& scene) const
		{
			// Todo...
			return true;
		}
	};

	struct Image final
	{
		bool operator==(const Image& image) const
		{
			// Todo...
			return true;
		}
	};

	using ImageCache = influx::Cache<Image, string, ImageLoadDesc>;
}

// Define ImageLoadDesc Hash function...
namespace std
{
	template <>
	struct std::hash<influx::Assets::ImageLoadDesc>
	{
		size_t operator()(const influx::Assets::ImageLoadDesc& key) const noexcept
		{
			return 0u;
		}
	};
}