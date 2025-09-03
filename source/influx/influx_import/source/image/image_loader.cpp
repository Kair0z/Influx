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
	result<image_data> load_image_file(const string& filepath, const image_load_args& args)
	{
		using result_type = result<image_data>;

		vector<byte> data{};
		image_data out_image{};
		bool error = lodepng::decode(
			data,
			(unsigned&)out_image.m_dimensions.x,
			(unsigned&)out_image.m_dimensions.y,
			filepath.c_str(), 
			LCT_RGBA, 
			8u);

		if (error)
			return result_type::make_error("lodepng::decode() failed!");

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

		out_image.m_bytesize = out_image.m_pixels.size() * sizeof(uint32);
		return out_image;
	}

	/* Loads a 3D-image (cubemap) */
	result<cubemap_data> load_cubemap(const string& path, const cubemap_load_args& args)
	{
		using result_type = result<cubemap_data>;

		imp::image_data side_datas[6u]{};
		imp::image_load_args side_args{};

		bool all_success = true;
		for (uint32 i = 0u; i < 6u; ++i)
		{
			result<image_data> load_image_result = load_image_file(args.m_hacky_paths[i], side_args);
			if (load_image_result.is_success())
			{
				side_datas[i] = load_image_result.get();
			}
			else
			{
				all_success = false;
			}
		}

		if (!all_success)
			return result_type::make_error("not all images loaded correctly!");

		cubemap_data out_cubemap{};
		out_cubemap.m_dimensions.x =
			out_cubemap.m_dimensions.y = side_datas[0].m_dimensions.x;

		out_cubemap.m_bytesize = 6u * side_datas[0].m_bytesize;

		for (uint32 i = 0u; i < 6u; ++i)
		{
			for (uint64 p = 0u; p < side_datas[i].m_pixels.size(); ++p)
				out_cubemap.m_pixels.push_back(side_datas[i].m_pixels[p]);
		}

		return out_cubemap;
	}
}