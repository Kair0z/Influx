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

	enum class e_compile_debug_level : uint8
	{
		release,
		debug,
		num
	};

	/* */
	struct compile_args final
	{
	public:
		e_shader_target			m_target;
		e_shader_platform		m_platform;
		vector<string>			m_defines;
		vector<string>			m_add_args{};
		string					m_pdb_folder;
		string					m_pdb_filename;
		string					m_include_folder;
		e_compile_debug_level	m_debug_level;

		bool m_reflection_enabled;
		bool m_pbd_enabled;

		inline bool is_non_debug()
		{ return m_debug_level == e_compile_debug_level::release; }

		inline compile_args& add_define(const string& define) 
		{ m_defines.push_back(define); return *this; }
		
		inline compile_args& set_target(e_shader_target target) 
		{ m_target = target; return *this; }

		inline compile_args& set_platform(e_shader_platform platform) 
		{ m_platform = platform; return *this; }

		inline compile_args& set_pdb_folder(const string& folder)
		{ m_pdb_folder = folder; return *this; }

		inline compile_args& set_pdb_fname(const string& name)
		{ m_pdb_filename = name; return *this; }

		inline compile_args& set_include_folder(const string& folder)
		{ m_include_folder = folder; return *this; }

		inline compile_args& set_debug_level(const bool enabled)
		{ m_debug_level = enabled ? e_compile_debug_level::debug : e_compile_debug_level::release; return *this; }

		inline compile_args& set_debug_level(const e_compile_debug_level lvl)
		{ m_debug_level = lvl; return *this; }

		inline compile_args& set_reflection_enabled(const bool enabled)
		{ m_reflection_enabled = enabled; return *this; }

		inline compile_args& set_pdb_enabled(const bool enabled)
		{ m_pbd_enabled = enabled; return *this; }
	};

	/* */
	struct reflection final
	{
		// inputs & outputs
		struct io_param final
		{
			enum class e_component_type : uint8
			{
				unknown,
				f16,
				f32,
				u16,
				u32,
				num
			};

			enum class e_system_name : uint8
			{
				unknown,
				target,			// D3D_NAME_TARGEt (SV_TARGET)
				position,		// D3D_NAME_POSITION (SV_POSITION)
				num
			};

			string				m_semantic_name;
			uint32				m_semantic_index;
			e_system_name		m_system_name;
			e_component_type	m_component_type;
			uint32				m_num_floats;
			bool				m_is_input;
		};

		struct resource final
		{
			enum class e_type : uint8
			{
				unknown,
				rootconstants,
				constbuffer,
				structbuff,
				structbuff_rw,
				structbuff_append,
				structbuff_consume,
				structbuff_wcounter,
				byteaddress,
				byteaddress_rw,
				texture_rw,
				texture,
				sampler,
				accstruct,
				feedback_rw,		// D3D_SIT_UAV_FEEDBACKTEXTURE
				count
			};

			string m_name{};
			e_type m_type{};
			uint32 m_shader_register{};
			uint32 m_register_space{};

			inline bool is_texture() const
			{
				return m_type == e_type::texture
					|| m_type == e_type::texture_rw;
			}
			inline bool is_buffer() const
			{
				return m_type == e_type::constbuffer
					|| m_type == e_type::rootconstants
					|| m_type == e_type::structbuff
					|| m_type == e_type::structbuff_rw
					|| m_type == e_type::structbuff_append
					|| m_type == e_type::structbuff_consume
					|| m_type == e_type::structbuff_wcounter
					|| m_type == e_type::byteaddress
					|| m_type == e_type::byteaddress_rw;
			}

			// if cbv
			uint64 m_bytesize{};

			// if srv / sampler, this is the number of descriptors possibly used
			uint32 m_range_size = 0u;
		};

		vector<io_param> m_output_params{};
		vector<io_param> m_input_params{};
		vector<resource> m_bound_resources{};
	};

	/* */
	struct compile_output final
	{
		shader_signature	m_signature;
		vector<byte>		m_bytecode;
		reflection			m_reflection;
		vector<string>		m_log;
		bool m_success		= false;
	};

	/* */
	struct parse_output final
	{
		struct per_shader final
		{
			shader_signature m_signature;
		};

		uint32 get_total_num_shaders() const
		{
			uint32 result{};
			for (const auto& pair : m_shadermap)
				result += (uint32)pair.second.size();
			return result;
		}

		const vector<per_shader>& get_shadermap(e_shader_type type) const
		{
			return m_shadermap.at(type);
		}

		void merge(const parse_output& other)
		{
			m_found_types |= other.m_found_types;
			for (const auto& pair : other.m_shadermap)
				for (const auto& shader : pair.second)
				{
					m_shadermap[pair.first].push_back(shader);
				}
		}

		bool contains(e_shader_type type) const
		{
			return has_flag(m_found_types, get_shader_flag(type) );
		}

		e_shader_type_flags						m_found_types;
		map<e_shader_type, vector<per_shader>>	m_shadermap{};
	};

	/* finds & compiles a shader (based on args) from a.hlsl filepath */
	INFLUX_SHADER_API 
	result<compile_output> compile_shader_in_file(
		const string& filepath,
		const shader_signature& signature,
		const compile_args& args);

	/* finds & compiles a shader (based on args) from a string */
	INFLUX_SHADER_API 
	result<compile_output> compile_shader_in_source(
		const string& shader_source,
		const shader_signature& signature,
		const compile_args& args);
	
	/* 
		finds & parses an .hlsl file 
		to detect shaders it contains without compiling them 
	*/
	INFLUX_SHADER_API
	result<parse_output> parse_shaders_in_file(const string& filepath);

	/* 
		finds & parses all shaders in a given string
	*/
	INFLUX_SHADER_API 
	result<parse_output> parse_shaders_in_source(const string& shader_source);

	/* 
		compiling a shader library 
	*/
	struct shader_library_compile_args final
	{
		e_shader_target m_target;
		string m_entrypoint;
	};
	struct shader_library final
	{
		vector<byte> m_bytecode;

		vector<shader_signature> m_shader_signatures{};
		
		vector<string> m_log;

		bool m_success = false;
	};

	INFLUX_SHADER_API
	result<shader_library> compile_shader_library(const string& filepath, const shader_library_compile_args& args);
}