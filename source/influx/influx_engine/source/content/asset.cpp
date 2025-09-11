#include "engine_pch.h"
#include "asset.h"

// influx::imp
#include "influx_import.h"

namespace influx::engine
{
	result<imp::scene_data> load_scene_data(const string& path, const imp::scene_load_args& args)
	{
		return imp::load_scene_file(path, args);
	}
	result<imp::image_data> load_image_data(const string& path, const imp::image_load_args& args)
	{
		return imp::load_image_file(path, args);
	}
	result<imp::cubemap_data> load_cubemap_data(const string& path, const imp::cubemap_load_args& args)
	{
		return imp::load_cubemap(path, args);
	}
	result<shader_vector> load_shader_data(const string& path, const imp::shader_load_args& args)
	{
		return imp::load_shaders_in_file(path, args);
	}
}