#pragma once 
#include "core/math/colour.h"

namespace influx::renderer
{
	enum class e_texture_slot : uint8
	{
		albedo,
		normal,
		roughness,
		special,
		count
	};

	struct material final
	{
		// shaders:
		string m_vertex_shader = "";
		string m_pixel_shader = "";
		// ...

		// variables:
		math::colour_rgba m_basecolor;

		// this section sketches the pipeline object,
		bool m_enable_depth = true;
		bool m_enable_stencil = false;
		e_cull_mode m_cull_mode = e_cull_mode::back;
		// ...

		// this section sketches the rootsignature,
		// which is hard-coded to 4 resources...
		// textures:
		string m_tex_albedo;
		string m_tex_normal;
		string m_tex_roughness;
		string m_tex_special;

		string get_texture(e_texture_slot slot) const
		{
			switch (slot)
			{
			case e_texture_slot::albedo: return m_tex_albedo;
			case e_texture_slot::normal: return m_tex_normal;
			case e_texture_slot::roughness: return m_tex_roughness;
			case e_texture_slot::special: return m_tex_special;
			}

			return "";
		}
	};
}