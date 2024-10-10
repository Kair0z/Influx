#include "import_pch.h"
#include "influx_import.h"

#include "lodepng/lodepng.h"

namespace influx::imp
{
	LodePNGColorType translate(e_image_colour_type type)
	{
		switch (type)
		{
		case e_image_colour_type::grey: return LCT_GREY;
		case e_image_colour_type::rgb: return LCT_RGB;
		case e_image_colour_type::rgba: return LCT_RGBA;
		default:
		case e_image_colour_type::count: return LCT_RGBA;
		}
	}

	/* Loads an 2D-image (.png, .jpeg) */
	bool load_image_file(const string& filepath, image_data& out_image, const image_load_args& args)
	{
		vector<byte> data{};

		bool error = lodepng::decode(
			data,
			(unsigned&)out_image.m_dimensions.x,
			(unsigned&)out_image.m_dimensions.y,
			filepath.c_str(), 
			LCT_RGBA, 
			8u);

		constexpr uint32 k_num_channels = 4u;
		for (uint32 i = 0u; i < data.size() / k_num_channels; ++i)
		{
			out_image.m_pixels.push_back({});
			pixel32& pixel = out_image.m_pixels.back();
			
			uint32 r = data[(i * k_num_channels) + 0u];
			uint32 g = data[(i * k_num_channels) + 1u];
			uint32 b = data[(i * k_num_channels) + 2u];
			uint32 a = data[(i * k_num_channels) + 3u];

			pixel = make_pixel32(r, g, b, a);
		}

		return error;
	}
}