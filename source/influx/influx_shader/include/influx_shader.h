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
#include "core/pointer.h"

#define INFLUX_SHADER_BACKEND_SLANG 0
#define INFLUX_SHADER_BACKEND_DXC	1

namespace influx::shader
{
	template <typename _t = bool>
	using result = influx::result<_t, const char*>;

	enum class e_compile_debug_level : uint8
	{
		release,
		debug,
		num
	};

	using bytecode = vector<byte>;

	/* */
	struct compile_args final
	{
	public:
		e_shader_target			m_target;
		e_shader_platform		m_platform;
		e_shader_language		m_source_language;
		e_shader_binary_output	m_output_format;
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
		enum class e_resource_type : uint8
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
			target,			// D3D_NAME_TARGET (SV_TARGET)
			position,		// D3D_NAME_POSITION (SV_POSITION)
			num
		};

		using parmblock_id = uint32;
		using ioparam_id = uint32;
		using entrypoint_id = uint32;

		static const uint32 k_name_fixed_length = 64u;
		static const uint32 k_parmblock_invalid = (uint32)-1;
		using name = char[k_name_fixed_length];

		struct io_param final
		{
			name				m_typename;
			name				m_name;
			uint32				m_semantic_index;
			e_system_name		m_system_name;
			e_component_type	m_component_type;
			uint32				m_num_floats;
			bool				m_is_input;
		};
		struct resource final
		{
			name	m_name;
			uint32	m_register_index; // shader register
			uint32	m_register_space;
			uint32	m_arraysize;
			uint32	m_parent_block_index = k_parmblock_invalid;
			uint32	m_nested_block_index = k_parmblock_invalid;

			e_resource_type m_type{};
			uint32 m_shader_register{};
			uint64 m_bytesize{};
			
			inline bool is_texture() const
			{
				return m_type == e_resource_type::texture
					|| m_type == e_resource_type::texture_rw;
			}
			inline bool is_buffer() const
			{
				return m_type == e_resource_type::constbuffer
					|| m_type == e_resource_type::rootconstants
					|| m_type == e_resource_type::structbuff
					|| m_type == e_resource_type::structbuff_rw
					|| m_type == e_resource_type::structbuff_append
					|| m_type == e_resource_type::structbuff_consume
					|| m_type == e_resource_type::structbuff_wcounter
					|| m_type == e_resource_type::byteaddress
					|| m_type == e_resource_type::byteaddress_rw;
			}
		};
		struct parmblock final
		{
			name	m_name;
			uint32	m_resource_start_index;
			uint32	m_resource_num;
			uint32	m_parent_block_index = k_parmblock_invalid;
		};

		e_shader_type m_shader_type = e_shader_type::count;
		vector<io_param> m_ioparams{};
		vector<resource> m_bound_resources{};
		vector<resource> m_resources;
		vector<parmblock> m_parmblocks;

		static void set_name(name old, const string& new_name) {
			_strnset_s(old, sizeof(name), 0, k_name_fixed_length);
			strncpy_s(old, sizeof(name), new_name.c_str(), new_name.size());
		}

		parmblock& add_parmblock() {
			m_parmblocks.push_back({});
			return m_parmblocks.back();
		}
		resource& add_resource(parmblock_id parmblock_id = k_parmblock_invalid)  {
			m_parmblocks[parmblock_id].m_resource_num++;
			m_resources.push_back({});
			return m_resources.back();
		}
		io_param& add_ioparam(entrypoint_id entry_id = k_parmblock_invalid) {
			m_ioparams.push_back({});
			return m_ioparams.back();
		}

		INFLUX_SHADER_API static void serialize(const reflection& refl, std::ostream& out);
		INFLUX_SHADER_API static void deserialize(reflection& refl, std::istream& in);
		INFLUX_SHADER_API static void deserialize(reflection& refl, const byte* bytes, const uint64 size);
	};

	/* */
	struct compile_output final
	{
		shader_signature	m_signature;
		bytecode			m_bytecode;
		reflection			m_reflection;
		vector<string>		m_log;
		bool m_success		= false;
	};

	/* programs are bundles of shaders */
	struct compile_program_output final
	{
		vector<shader_signature> m_signatures{};
		vector<bytecode>		 m_entrypoint_codeblobs{};
		reflection				 m_reflection{};
		vector<string>			 m_log{};
		bool					 m_success = false;
	};

	struct compiled_rootsignature final
	{
		bytecode m_bytecode;
	};

	/* */
	struct parse_output final
	{
		struct per_shader final
		{
			shader_signature m_signature;
		};
		using shadermap = map<e_shader_type, vector<per_shader>>;

		e_shader_type_flags	m_found_types;
		shadermap			m_shadermap{};

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

		cptr<per_shader> get_first_shader() const
		{
			for (const auto& pair : m_shadermap)
				for (const auto& shader : pair.second)
				{
					return &shader;
				}
			return nullptr;
		}

		cptr<per_shader> find_shader_by_entrypoint(const string& entrypoint) const
		{
			for (const auto& pair : m_shadermap)
				for (const auto& shader : pair.second)
				{
					if (shader.m_signature.get_entrypoint() == entrypoint)
						return &shader;
				}
			return nullptr;
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
	};

	struct reflect_output final
	{
		reflection m_reflection;
	};

	static constexpr const char* k_valid_file_extensions[]
	{
		".hlsl",
		".slang"
	};
	
	static const bool is_file_extension_valid(const string& extension)
	{
		static constexpr int k_num_valid_file_extensions = _countof(k_valid_file_extensions);
		for (int i = 0u; i < k_num_valid_file_extensions; ++i)
		{
			static constexpr bool case_sensitive = true;
			if ( string::is_equal(extension, k_valid_file_extensions[i], case_sensitive ) )
				return true;
		}
		return false;
	}

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

	INFLUX_SHADER_API
	result<parse_output> parse_shaders_in_folder(const string& folderpath, const bool recursive, const char* file_extension = k_valid_file_extensions[0]);

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