#pragma once

#if _DLL
	#define INFLUX_SHADER_API __declspec(dllexport)
#else
	#define INFLUX_SHADER_API __declspec(dllimport)
#endif

#include "core/basetypes.h"
#include "core/string.h"
#include "core/container/vector.h"

namespace influx::shader
{
	enum class e_shader_type : uint8
	{
		vs,
		ps,
		count
	};

	enum class e_shader_target : uint8
	{
		_6_2,
		count
	};

	struct compile_args final
	{
	public:
		inline void add_define(const string& define)
		{
			m_defines.push_back(define);
		}

		e_shader_type m_type;
		e_shader_target m_target;
		string m_entrypoint;
		vector<string> m_defines;

		bool m_compile_debug;
		bool m_reflection;
		bool m_pbd;
	};

	struct compile_output final
	{
		vector<byte> m_bytecode;
	};

	// compiles from a .hlsl filepath
	INFLUX_SHADER_API compile_output compile_shader(const string& filepath, const compile_args& args);

	// compiles from text source in a string
	INFLUX_SHADER_API compile_output compile_shader_source(const string& shader_source, const compile_args& args);
}