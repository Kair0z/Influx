#pragma once

#if _DLL
	#define INFLUX_SHADER_API __declspec(dllexport)
#else
	#define INFLUX_SHADER_API __declspec(dllimport)
#endif

#include "core/basetypes.h"
#include "core/string.h"
#include "core/container/vector.h"
#include "core/shader.h"
;
namespace influx::shader
{
	struct compile_args final
	{
	public:
		inline void add_define(const string& define)
		{
			m_defines.push_back(define);
		}

		inline bool is_valid() const
		{
			return m_signature.is_valid();
		}

		shader_signature m_signature;

		vector<string> m_defines;
		string m_pdb_folder;
		string m_pdb_filename;
		string m_include_folder;

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
				structured,
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
		shader_signature m_signature;
		vector<byte> m_bytecode;
		reflection m_reflection;
		vector<string> m_log;
		bool m_success = false;
	};

	// compiles from a .hlsl filepath
	INFLUX_SHADER_API compile_output compile_shader(const string& filepath, const compile_args& args);

	// compiles from text source in a string
	INFLUX_SHADER_API compile_output compile_shader_source(const string& shader_source, const compile_args& args);

	// parses information about a shader without compiling
	struct parse_output final
	{
		struct per_shader final
		{
			shader::e_shader_type m_type;
			string m_entrypoint;

			// partially filled compile args (containing type, filename, entrypoint)
			compile_args m_compile_args;
		};
		vector<per_shader> m_shaders;
	};

	INFLUX_SHADER_API parse_output parse_shaderfile(const string& filepath, const compile_args& args);
	INFLUX_SHADER_API parse_output parse_shader_source(const string& shader_source, const compile_args& args);
}