#pragma once

#if _DLL
	#define INFLUX_SHADER_API __declspec(dllexport)
#else
	#define INFLUX_SHADER_API __declspec(dllimport)
#endif

// influx::core
#include "core/result.h"
#include "core/basetypes.h"
#include "core/string.h"
#include "core/container/vector.h"
#include "core/shader.h"

namespace influx::shader
{
	template <typename _t>
	using result = influx::result<_t, const char*>;

	/* */
	struct compile_args final
	{
	public:
		inline bool is_valid() const
		{
			return m_signature.is_valid();
		}

		inline compile_args& add_define(const string& define) { m_defines.push_back(define); return *this; }
		inline compile_args& set_target(e_shader_target target) { m_signature.m_target = target; return *this; }
		inline compile_args& set_type(e_shader_type type) { m_signature.m_type = type; return *this; }
		
		shader_signature m_signature;

		vector<string> m_defines;
		string m_pdb_folder;
		string m_pdb_filename;
		string m_include_folder;

		bool m_compile_debug;
		bool m_reflection;
		bool m_pbd;
	};

	/* */
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

	/* */
	struct compile_output final
	{
		shader_signature m_signature;

		vector<byte> m_bytecode;

		reflection m_reflection;
		
		vector<string> m_log;

		bool m_success = false;
	};

	// parses information about a shader without compiling
	struct parse_output final
	{
		shader_signature m_signature;

		inline const string& get_entrypoint() const
		{
			return m_signature.m_entrypoint;
		}

		inline const shader::e_shader_type& get_shader_type() const
		{
			return m_signature.m_type;
		}
	};

	/* finds & compiles a shader (based on args) from a.hlsl filepath */
	INFLUX_SHADER_API 
	result<compile_output> compile_shader_in_file(const string& filepath, const compile_args& args);

	/* finds & compiles a shader (based on args) from a string */
	INFLUX_SHADER_API 
	result<compile_output> compile_shader(const string& shader_source, const compile_args& args);
	
	/* finds & parses an .hlsl file to detect shaders it contains without compiling them */
	INFLUX_SHADER_API 
	result<vector<parse_output>> parse_shaders_in_file(const string& filepath);

	/* finds & parses all shaders in a given string */
	INFLUX_SHADER_API 
	result<vector<parse_output>> parse_shader(const string& shader_source);

	struct shader_library_compile_args final
	{
		e_shader_target m_target;
	};

	struct shader_library final
	{
		vector<byte> m_bytecode;
		vector<string> m_log;

		bool m_success = false;
	};

	INFLUX_SHADER_API
	result<shader_library> compile_shader_library(const string& filepath, const shader_library_compile_args& args);
}