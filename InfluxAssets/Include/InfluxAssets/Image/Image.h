#pragma once

#if INFLUX_ASSETS_USE_CORE
#include "Core/Cache.h"
#include "Core/Container/Map.h"
#endif

namespace Influx::Assets
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

	using ImageCache = Influx::Cache<Image, String, ImageLoadDesc>;
}

// Define ImageLoadDesc Hash function...
namespace std
{
	template <>
	struct std::hash<Influx::Assets::ImageLoadDesc>
	{
		size_t operator()(const Influx::Assets::ImageLoadDesc& key) const noexcept
		{
			return 0u;
		}
	};
}