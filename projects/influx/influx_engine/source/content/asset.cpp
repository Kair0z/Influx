#include "engine_pch.h"
#include "asset.h"

// influx::imp
#include "influx_import.h"

// influx::shader
#include "influx_shader.h"

namespace influx::engine
{
	imp::scene_data load_scene_data(const string& path, const imp::scene_load_args& args)
	{
		imp::scene_data data{};
		imp::load_scene_file(path, data, args);
		return data;
	}
	imp::image_data load_image_data(const string& path, const imp::image_load_args& args)
	{
		imp::image_data data{};
		imp::load_image_file(path, data, args);
		return data;
	}
	imp::cubemap_data load_cubemap_data(const string& path, const imp::cubemap_load_args& args)
	{
		imp::cubemap_data data{};
		imp::load_cubemap(path, data, args);
		return data;
	}
	imp::shader_data load_shader_data(const string& path, const shader::compile_args& args)
	{
		imp::shader_data result{};
		bool success = imp::load_shader_file(path, result, args);
		return result;
	}
}