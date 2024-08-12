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

	struct reflection final
	{
		struct input_param final
		{
			string m_semantic_name;
			uint32 m_semantic_index;
			uint32 m_num_floats;
		};

		struct resource final
		{
			enum class e_type : uint8
			{
				cbv,
				sampler,
				uav,
				srv,
				texture,
				unknown,
				count
			};

			string m_name{};
			e_type m_type{};
			uint32 m_shader_register{};
			uint32 m_register_space{};

			// if cbv
			uint64 m_bytesize{};

			// if srv / sampler, this is the number of descriptors possibly used
			uint32 m_range_size = 0u;
		};

		vector<input_param> m_input_params{};
		vector<resource> m_bound_resources{};
	};

	struct compile_output final
	{
		vector<byte> m_bytecode;
		reflection m_reflection;
	};

	// compiles from a .hlsl filepath
	INFLUX_SHADER_API compile_output compile_shader(const string& filepath, const compile_args& args);

	// compiles from text source in a string
	INFLUX_SHADER_API compile_output compile_shader_source(const string& shader_source, const compile_args& args);
}