#pragma once

// influx::core
#include "basetypes.h"
#include "core/enum.h"

namespace influx::shader
{
	/* supported shader types */
	enum class e_shader_type : uint8
	{
		// graphics
		vs,				// vertex shader
		ps,				// pixel shader
		ds,				// domain shader
		gs,				// geometry shader
		hs,				// hull shader

		// compute
		cs,

		// raytracing
		rgs,			// ray-gen shader
		mss,			// miss shader
		chs,			// closest hit shader
		ahs,			// any hit shader
		ins,			// intersection shader

		// mesh shading
		as,				// amplification shader
		ms,				// mesh shader

		count
	};
	static constexpr uint8 k_num_shadertypes = static_cast<uint8>(e_shader_type::count);
	static const char* k_shadertype_strings[k_num_shadertypes]
	{
		"vs",
		"ps",
		"ds",
		"gs",
		"hs",
		"cs",
		"rgs",
		"mss",
		"chs",
		"ahs",
		"ins",
		"as",
		"ms"
	};
	static const char* k_shadertype_friendnames[k_num_shadertypes]
	{
		"vertex",
		"pixel",
		"domain",
		"geometry",
		"hull",
		"compute",
		"raygen",
		"miss",
		"closest_hit",
		"any_hit",
		"intersection",
		"amplification",
		"mesh"
	};

	enum class e_shader_type_flags : uint32
	{
		none	= 0,

		vs		= 1 << static_cast<uint8>(e_shader_type::vs),
		ps		= 1 << static_cast<uint8>(e_shader_type::ps),
		ds		= 1 << static_cast<uint8>(e_shader_type::ds),
		gs		= 1 << static_cast<uint8>(e_shader_type::gs),
		hs		= 1 << static_cast<uint8>(e_shader_type::hs),
		cs		= 1 << static_cast<uint8>(e_shader_type::cs),
		rgs		= 1 << static_cast<uint8>(e_shader_type::rgs),
		mss		= 1 << static_cast<uint8>(e_shader_type::mss),
		chs		= 1 << static_cast<uint8>(e_shader_type::chs),
		ahs		= 1 << static_cast<uint8>(e_shader_type::ahs),
		ins		= 1 << static_cast<uint8>(e_shader_type::ins),
		as		= 1 << static_cast<uint8>(e_shader_type::as),
		ms		= 1 << static_cast<uint8>(e_shader_type::ms),

		all_gfx		= vs | ps | ds | gs | hs,
		all_cs		= cs,
		all_ray		= rgs | mss | chs | ahs | ins,
		all_mesh	= as | ms,

		all = all_gfx | all_cs | all_ray | all_mesh
	};

	inline static constexpr e_shader_type_flags get_shader_flag(e_shader_type type)
	{
		return static_cast<e_shader_type_flags>(1u << static_cast<uint8>(type));
	}

	/* pipeline shader groups */
	inline static constexpr bool is_compute_shader(e_shader_type type)
	{
		return type == e_shader_type::cs;
	}

	inline static constexpr bool is_graphics_shader(e_shader_type type)
	{
		return
			type == e_shader_type::vs ||
			type == e_shader_type::ps ||
			type == e_shader_type::ds ||
			type == e_shader_type::gs ||
			type == e_shader_type::hs;
	}

	inline static constexpr bool is_raytracing_shader(e_shader_type type)
	{
		return
			type == e_shader_type::rgs ||
			type == e_shader_type::mss ||
			type == e_shader_type::chs ||
			type == e_shader_type::ahs ||
			type == e_shader_type::ins;
	}

	inline static constexpr bool is_mesh_shader(e_shader_type type)
	{
		return
			type == e_shader_type::as ||
			type == e_shader_type::ms;
	}

	/* supported shader targets */
	enum class e_shader_target : uint8
	{
		_6_2,
		_6_5,
		_6_6,
		_6_8,
		count
	};
	static constexpr uint8 k_num_shadertargets = static_cast<uint8>(e_shader_target::count);
	static const char* k_shadertarget_strings[k_num_shadertargets]
	{
		"6_2",
		"6_5",
		"6_6",
		"6_8"
	};

	/*	
		signature identifier of a single shader
		built from filename+entrypoint+shader+target
	*/
	using shader_id = uint64;
	struct shader_signature final
	{
		/* tag & id are derived */
		string			m_tag;
		shader_id		m_id;

		/* these 4 components make the identifier */
		e_shader_type		m_type;
		e_shader_target		m_target;
		string				m_entrypoint;
		string				m_filename;

		bool is_valid() const
		{
			return m_type != e_shader_type::count
				&& m_target != e_shader_target::count
				&& !m_entrypoint.empty()
				&& !m_filename.empty();
		}

		const string& get_tag() const
		{
			return m_tag;
		}

		shader_id get_id() const
		{
			return m_id;
		}

		void cache_id()
		{
			// vs6_6
			const string type_str = k_shadertype_strings[static_cast<uint8>(m_type)];
			const string targ_str = k_shadertarget_strings[static_cast<uint8>(m_target)];

			// filename::entrypoint::vs6_6
			m_tag = m_filename + "::" + m_entrypoint + "::" + type_str + targ_str;
			m_id = std::hash<string>{}(m_tag);
		}

		bool operator==(const shader_signature& other) const
		{
			return m_id == other.m_id;
		}
	};
}
ENABLE_ENUM_BIT_OPERATORS(influx::shader::e_shader_type_flags);

// Specialize std::hash for shader_signature
namespace std 
{
	template <> struct hash<influx::shader::shader_signature> 
	{
		std::size_t operator()(const influx::shader::shader_signature& sig) const 
		{
			return sig.get_id();
		}
	};
}