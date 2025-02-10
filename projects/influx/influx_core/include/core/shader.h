#pragma once

// influx::core
#include "basetypes.h"

namespace influx::shader
{
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
		"ins"
	};

	enum class e_shader_target : uint8
	{
		_6_2,
		_6_5,
		_6_6,
		count
	};
	static constexpr uint8 k_num_shadertargets = static_cast<uint8>(e_shader_target::count);
	static const char* k_shadertarget_strings[k_num_shadertargets]
	{
		"6_2",
		"6_5",
		"6_6"
	};

	using shader_id = uint64;

	struct shader_signature final
	{
		bool is_valid() const
		{
			return m_type != e_shader_type::count
				&& m_target != e_shader_target::count
				&& !m_entrypoint.empty()
				&& !m_filename.empty();
		}

		string get_tag()
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

		e_shader_type m_type;
		e_shader_target m_target;
		string m_entrypoint;
		string m_filename;
		string m_tag;
		shader_id m_id;

		bool operator==(const shader_signature& other) const
		{
			return m_id == other.m_id;
		}
	};
}

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