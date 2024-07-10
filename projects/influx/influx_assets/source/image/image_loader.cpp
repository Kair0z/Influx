#include "assets_pch.h"
#include "influx_assets.h"

#include "lodepng/lodepng.h"

namespace influx::assets
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
		bool error = lodepng::decode(
			out_image.m_pixels,
			(unsigned&)out_image.m_dimensions.x,
			(unsigned&)out_image.m_dimensions.y,
			filepath.c_str(), 
			LCT_RGBA, 
			8u);

		return error;
	}
}