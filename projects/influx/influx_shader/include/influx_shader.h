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
		// graphics
		vs,
		ps,
		ds,
		gs,
		hs,

		// compute
		cs,

		// raytracing

		//
		count
	};
	static constexpr uint8 k_num_shadertypes = static_cast<uint8>(e_shader_type::count);

	enum class e_shader_target : uint8
	{
		_6_2,
		_6_5,
		_6_6,
		count
	};
	static constexpr uint8 k_num_shadertargets = static_cast<uint8>(e_shader_target::count);

	struct shader_type_target final
	{
		e_shader_type m_type;
		e_shader_target m_target;
	};

	struct shader_signature final
	{
		bool is_valid() const
		{
			return m_type != e_shader_type::count
				&& m_target != e_shader_target::count
				&& !m_entrypoint.empty()
				&& !m_path.empty();
		}

		e_shader_type m_type;
		e_shader_target m_target;
		string m_entrypoint;
		string m_path;

		bool operator==(const shader_signature&) const = default; // Automatically generates an equality operator
	};

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
}

// Specialize std::hash for shader_signature
namespace std {
	template <>
	struct hash<influx::shader::shader_signature> {
		std::size_t operator()(const influx::shader::shader_signature& sig) const {
			return 
				std::hash<influx::string>{}(sig.m_entrypoint)						 << 0 ^ 
				std::hash<influx::uint8>{}(static_cast<influx::uint8>(sig.m_type))	 << 1 ^
				std::hash<influx::uint8>{}(static_cast<influx::uint8>(sig.m_target)) << 2;

			//std::hash<influx::string>{}(sig.m_path) ^
		}
	};
}