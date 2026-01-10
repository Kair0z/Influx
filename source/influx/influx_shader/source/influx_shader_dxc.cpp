#include "influx_shader.h"

#if INFLUX_SHADER_BACKEND_DXC
#include "core/file.h"
#include "core/container/map.h"
#include "core/regex.h"

// https://strontic.github.io/xcyclopedia/library/dxc.exe-0C1709D4E1787E3EB3E6A35C85714824.html
// dx12 compiler
// #include "dxc/d3dcompiler.h"
#include <Windows.h>
#include "dxc/dxcapi.h"
#include "dxc/d3d12shader.h"
#pragma comment (lib, "dxcompiler.lib")
#pragma comment (lib, "dxil.lib")

// global dxc utils
static inline IDxcUtils* get_utils()
{
	static IDxcUtils* g_pUtils = nullptr;
	if (g_pUtils == nullptr)
	{
		HRESULT res = ::DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&g_pUtils));
	}
	return g_pUtils;
}

namespace influx::shader
{
	class influx_include_handler : public IDxcIncludeHandler
	{
	public:
		void set_include_folder(const string& path)
		{
			m_include_folder = path;
		}

		HRESULT STDMETHODCALLTYPE LoadSource(_In_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) override
		{
			IDxcBlobEncoding* pEncoding;
			string filename = to_string( path(pFilename).get_full_path() );
			
			if (m_included_map.contains(filename))
			{
				// Return empty string blob if this file has been included before
				static const char nullStr[] = " ";
				get_utils()->CreateBlobFromPinned(nullStr, ARRAYSIZE(nullStr), DXC_CP_ACP, &pEncoding);
				*ppIncludeSource = pEncoding;
				return S_OK;
			}

			// load the file and return the file as source
			HRESULT hr = get_utils()->LoadFile(pFilename, nullptr, &pEncoding);
			if (SUCCEEDED(hr))
			{
				m_included_map[filename] = filename;
				*ppIncludeSource = pEncoding;
			}
			else
			{
				auto rooted_path = m_include_folder + pFilename;
				hr = get_utils()->LoadFile(rooted_path.c_wstr(), nullptr, &pEncoding);
			}
			return hr;
		}

		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override { return E_NOINTERFACE; }
		ULONG STDMETHODCALLTYPE AddRef(void) override { return 0; }
		ULONG STDMETHODCALLTYPE Release(void) override { return 0; }

	private:
		umap<string, string> m_included_map{};
		string m_include_folder{};
	};

	/* e.g.: vs_6_2*/
	inline static string build_shader_target_string(e_shader_type type, e_shader_target target)
	{
		string result{};

		if (is_raytracing_shader(type))
		{
			// raytracing shaders msut be compiled as libs!!
			result += "lib";
		}
		else
		{
			result += shader::k_shadertype_strings[static_cast<uint32>(type)];
		}

		result += "_";
		result += shader::k_shadertarget_strings[static_cast<uint32>(target)];

		return result;
	}

	/* e.g.: vs_6_2*/
	inline static string build_shaderlib_target_string(e_shader_target target)
	{
		string result = "lib_";
		result += shader::k_shadertarget_strings[static_cast<uint32>(target)];
		return result;
	}

	inline uint32 calc_num_floats_from_mask(uint32 mask)
	{
		uint32 num_floats = 0u;
		while (mask) {
			num_floats += mask & 1;
			mask >>= 1;
		}
		return num_floats;
	}

	inline reflection::e_component_type get_component_type(D3D_REGISTER_COMPONENT_TYPE type)
	{
		switch (type)
		{
		// case D3D_REGISTER_COMPONENT_FLOAT16: return reflection::io_param::e_component_type::f16;
		case D3D_REGISTER_COMPONENT_FLOAT32: return reflection::e_component_type::f32;
		// case D3D_REGISTER_COMPONENT_UINT16:	 return reflection::io_param::e_component_type::u16;
		case D3D_REGISTER_COMPONENT_UINT32:	 return reflection::e_component_type::u32;
		}
		return reflection::e_component_type::unknown;
	}

	inline reflection::e_system_name get_system_name(D3D_NAME name)
	{
		switch (name)
		{
		case D3D_NAME_TARGET: return reflection::e_system_name::target;
		case D3D_NAME_POSITION: return reflection::e_system_name::position;
		default:
			return reflection::e_system_name::unknown;
		}
	}

	inline reflection::io_param translate(const D3D12_SIGNATURE_PARAMETER_DESC& desc)
	{
		reflection::io_param result{};
		reflection::set_name(result.m_name, desc.SemanticName);
		result.m_semantic_index = desc.SemanticIndex;
		result.m_component_type = get_component_type(desc.ComponentType);
		result.m_num_floats = calc_num_floats_from_mask(desc.Mask);
		result.m_system_name = get_system_name(desc.SystemValueType);
		return result;
	}

	static constexpr e_shader_type translate(D3D12_SHADER_VERSION_TYPE type)
	{
		switch (type)
		{
			case D3D12_SHVER_PIXEL_SHADER				: return e_shader_type::ps;
			case D3D12_SHVER_VERTEX_SHADER				: return e_shader_type::vs;
			case D3D12_SHVER_GEOMETRY_SHADER			: return e_shader_type::gs;
			case D3D12_SHVER_HULL_SHADER				: return e_shader_type::hs;
			case D3D12_SHVER_DOMAIN_SHADER				: return e_shader_type::ds;
			case D3D12_SHVER_COMPUTE_SHADER				: return e_shader_type::cs;
			case D3D12_SHVER_LIBRARY					: return e_shader_type::lib;
			case D3D12_SHVER_RAY_GENERATION_SHADER		: return e_shader_type::rgs;
			case D3D12_SHVER_INTERSECTION_SHADER		: return e_shader_type::ins;
			case D3D12_SHVER_ANY_HIT_SHADER				: return e_shader_type::ahs;
			case D3D12_SHVER_CLOSEST_HIT_SHADER			: return e_shader_type::chs;
			case D3D12_SHVER_MISS_SHADER				: return e_shader_type::mss;
			case D3D12_SHVER_CALLABLE_SHADER			: return e_shader_type::call;
			case D3D12_SHVER_MESH_SHADER				: return e_shader_type::ms;
			case D3D12_SHVER_AMPLIFICATION_SHADER		: return e_shader_type::as;
		}
		return {};
	}

	inline reflection reflect_shader(ID3D12ShaderReflection* dx12_refl)
	{
		reflection result{};
		
		HRESULT hres = {};
		D3D12_SHADER_DESC shader_desc{};
		hres = dx12_refl->GetDesc(&shader_desc);

		const D3D12_SHADER_VERSION_TYPE shader_type = (D3D12_SHADER_VERSION_TYPE)D3D12_SHVER_GET_TYPE(shader_desc.Version);
		result.m_shader_type = translate(shader_type);

		// get input parameters
		for (uint32 i = 0u; i < shader_desc.InputParameters; ++i)
		{
			D3D12_SIGNATURE_PARAMETER_DESC signatureParameterDesc{};
			hres = dx12_refl->GetInputParameterDesc(i, &signatureParameterDesc);

			const uint32 entrypoint_index = 0u;
			auto& new_param = result.add_ioparam(entrypoint_index);
			new_param = translate(signatureParameterDesc);
			new_param.m_is_input = true;
			
		}
		// get output parameters
		for (uint32 i = 0u; i < shader_desc.OutputParameters; ++i)
		{
			D3D12_SIGNATURE_PARAMETER_DESC desc{};
			hres = dx12_refl->GetOutputParameterDesc(i, &desc);

			const uint32 entrypoint_index = 0u;
			auto& new_param = result.add_ioparam(entrypoint_index);
			new_param = translate(desc);
			new_param.m_is_input = false;
		}

		// get bound resources
		for (uint32 i = 0u; i < shader_desc.BoundResources; ++i)
		{
			reflection::resource resource{};

			D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc{};
			hres = dx12_refl->GetResourceBindingDesc(i, &shaderInputBindDesc);
			resource.m_shader_register = shaderInputBindDesc.BindPoint;
			resource.m_register_space = shaderInputBindDesc.Space;
			resource.m_arraysize = shaderInputBindDesc.BindCount;
			reflection::set_name(resource.m_name, string(shaderInputBindDesc.Name));

			// get variable info
			D3D12_SHADER_VARIABLE_DESC variable_desc{};
			ID3D12ShaderReflectionVariable* variable_refl = dx12_refl->GetVariableByName(shaderInputBindDesc.Name);
			hres = variable_refl->GetDesc(&variable_desc);
			
			bool resource_added = false;
			switch (shaderInputBindDesc.Type)
			{
			case D3D_SHADER_INPUT_TYPE::D3D_SIT_STRUCTURED:
			{
				resource.m_type = reflection::e_resource_type::structbuff;
			}break;
			case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWSTRUCTURED:
			{
				resource.m_type = reflection::e_resource_type::structbuff_rw;
			}break;
			case D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE:
			{
				resource.m_type = reflection::e_resource_type::texture;
				// ...
			}break;
			case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWTYPED:
			{
				resource.m_type = reflection::e_resource_type::texture_rw;
			}break;
			case D3D_SHADER_INPUT_TYPE::D3D_SIT_CBUFFER:
			{
				D3D12_SHADER_BUFFER_DESC constantBufferDesc{};
				ID3D12ShaderReflectionConstantBuffer* cbuffer_refl = dx12_refl->GetConstantBufferByIndex(i);
				hres = cbuffer_refl->GetDesc(&constantBufferDesc);

				// can be either ROOT CONSTANTS
				static const char* k_rootglobals_str = "$Globals";
				string constbuffername = constantBufferDesc.Name;
				const bool is_rootvars = constbuffername.contains(k_rootglobals_str, false);
				if (is_rootvars)
				{
					for (uint32 i = 0u; i < constantBufferDesc.Variables; ++i)
					{
						reflection::resource rootvar_resource{};
						rootvar_resource.m_type = reflection::e_resource_type::rootconstants;
						ID3D12ShaderReflectionVariable* cbuffer_var = cbuffer_refl->GetVariableByIndex(i);
						if (cbuffer_var)
						{
							D3D12_SHADER_VARIABLE_DESC var_desc{};
							hres = cbuffer_var->GetDesc(&var_desc);

							// each c# register is 16 bytes (4 floats)
							static constexpr uint32 k_bytes_per_register = 16u;
							const uint32 implicit_register = var_desc.StartOffset / k_bytes_per_register;

							rootvar_resource.m_bytesize = var_desc.Size;
							reflection::set_name(rootvar_resource.m_name, var_desc.Name);
							rootvar_resource.m_register_space = 0;
							rootvar_resource.m_shader_register = implicit_register;
							result.m_bound_resources.push_back(rootvar_resource);
						}
					}
					resource_added = true;
				}
				else // or CBV resource
				{
					resource.m_type = reflection::e_resource_type::constbuffer;
					resource.m_bytesize = constantBufferDesc.Size;
				}
			}break;
			case D3D_SHADER_INPUT_TYPE::D3D_SIT_BYTEADDRESS:
			{
				resource.m_type = reflection::e_resource_type::byteaddress;
			}break;
			case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWBYTEADDRESS:
			{
				resource.m_type = reflection::e_resource_type::byteaddress_rw;
			}break;
			case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_APPEND_STRUCTURED:
			{
				resource.m_type = reflection::e_resource_type::structbuff_append;
			}break;
			case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_CONSUME_STRUCTURED:
			{
				resource.m_type = reflection::e_resource_type::structbuff_consume;
			}break;
			case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
			{
				resource.m_type = reflection::e_resource_type::structbuff_wcounter;
			}break;
			case D3D_SHADER_INPUT_TYPE::D3D_SIT_RTACCELERATIONSTRUCTURE:
			{
				resource.m_type = reflection::e_resource_type::accstruct;
			}break;
			case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_FEEDBACKTEXTURE:
			{
				resource.m_type = reflection::e_resource_type::feedback_rw;
			}break;
			case D3D_SHADER_INPUT_TYPE::D3D_SIT_SAMPLER:
			{
				resource.m_type = reflection::e_resource_type::sampler;
				// ...
			}break;
			default:
				resource.m_type = reflection::e_resource_type::unknown;
				// ...
				break;
			}

			if (!resource_added)
				result.m_bound_resources.push_back(resource);
		}

		return result;
	}

	inline static string make_shader_name_string(
		const shader_signature& signature,
		const compile_args& args)
	{
		string type = build_shader_target_string(signature.m_type, args.m_target);
		string entry = signature.m_entrypoint;
		return type + entry;
	}

	inline vector<string> make_compile_args_strings(
		const shader_signature& signature,
		const compile_args& args)
	{
		vector<string> result{};

		// entrypoint (-E)
		const string& entrypoint = signature.m_entrypoint;
		result.push_back( "-E ");
		result.push_back( entrypoint );
		result.push_back( " ");

		// exports (-exports)
		// (in case of raytracing, we compile a single shader as a shader lib and so declare a single export)
		const bool compile_as_shaderlib = shader::is_raytracing_shader(signature.m_type);
		if (compile_as_shaderlib)
		{
			result.push_back( "-exports ");
			result.push_back( entrypoint );
			result.push_back( " ");
		}

		// target (-T) (eg. ps_6_2)
		string profile = build_shader_target_string(signature.m_type, args.m_target);
		result.push_back( "-T ");
		result.push_back( profile );
		result.push_back( " ");

		// includes (-I)
		const string& include_folder = args.m_include_folder;
		result.push_back( "-I ");
		result.push_back( include_folder );
		result.push_back( " ");

		// SPIRV
		if (args.m_platform == e_shader_platform::SPIRV)
		{
			result.push_back(" -spirv ");
			result.push_back(" -fspv-target-env=vulkan1.3 ");
			// result.push_back(" -fspv-preserve-bindings ");
			// result.push_back(" -fspv-preserve-interface ");
			// result.push_back(" -fspv-debug=file ");
		}

		// defines (-D)
		for (const string& define : args.m_defines)
		{
			result.push_back( "-D ");
			result.push_back( define );
			result.push_back( " ");
		}

		// misc
		const bool compile_debug = args.m_debug_level == shader::e_compile_debug_level::debug;
		const bool row_major = true;
		result								.push_back("dxc -help | findstr Version");
		result								.push_back(row_major ? "-Zpr" : "Zpc");
		if (!args.m_pbd_enabled) result				.push_back("-Qstrip_debug");
		if (!args.m_reflection_enabled) result		.push_back("-Qstrip_reflect");
		if (compile_debug) result	.push_back("-Od"); // DXC_ARG_SKIP_OPTIMIZATIONS
		if (compile_debug) result	.push_back("-O0"); // DXC_ARG_OPTIMIZATION_LEVEL0
		if (compile_debug) result	.push_back("-Zi"); // DXC_ARG_DEBUG
		if (!args.m_add_args.empty())
		{
			for (const auto& arg : args.m_add_args) result.push_back(arg);
		}

		return result;
	}

	inline string make_compile_args_string(const compile_args& args)
	{
		return "";
	}

	inline result<IDxcCompiler3*> create_compiler()
	{
		using result_type = result<IDxcCompiler3*>;

		IDxcCompiler3* compiler = nullptr;
		HRESULT hres = ::DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
		if (hres != S_OK)
		{
			return result_type::make_error("error: DxcCreateInstance failed");
		}

		return compiler;
	}

	inline string get_compile_errors(IDxcResult& compile_result)
	{
		IDxcBlobUtf8* pErrors = nullptr;
		HRESULT hres = compile_result.GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
		if (hres != S_OK || (pErrors && pErrors->GetStringLength() > 0))
		{
			string all_errors_string = {};
			all_errors_string.append((char*)pErrors->GetBufferPointer());
			return all_errors_string;
		}

		return "";
	}

	inline result<vector<byte>> get_compile_bytecode(IDxcResult& compile_result)
	{
		using result_type = result<vector<byte>>;
		result_type result{};

		// [OUTPUT: RESULT SHADER BYTE CODE]
		IDxcBlob* pResultData = nullptr;
		IDxcBlobUtf16* pResultOutputName = nullptr;
		HRESULT hres = compile_result.GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pResultData), &pResultOutputName);
		if (hres == S_OK && pResultData)
		{
			vector<byte> bytecode{}; bytecode.reserve(pResultData->GetBufferSize());
			for (uint32 i = 0; i < pResultData->GetBufferSize(); ++i)
			{
				bytecode.push_back(reinterpret_cast<byte*>(pResultData->GetBufferPointer())[i]);
			}
			return bytecode;
		}
		else
		{
			return result_type::make_error("error: GetOutput(DXC_OUT_OBJECT) failed!");
		}
	}

	bool WriteBlobToFile(ID3DBlob* blob, const wchar_t* filename)
	{
		if (!blob || !filename)
			return false;

		// Open file in binary mode
		std::ofstream file(filename, std::ios::binary);
		if (!file)
			return false;

		// Write the blob contents
		file.write(static_cast<const char*>(blob->GetBufferPointer()), blob->GetBufferSize());
		file.close();

		return true;
	}

	inline result<compile_output> compile_shader_dxcbuffer(
		const DxcBuffer& buffer, 
		const shader_signature& signature, 
		const compile_args& args)
	{
		using result_type = result<compile_output>;
		result_type result = {};
		HRESULT hres{};

		// https://simoncoenen.com/blog/programming/graphics/DxcCompiling
		// create the Dxc Compiler
		auto res = create_compiler();
		if (res.is_fail())
			return result_type::make_error("error: DxcCreateInstance failed");

		IDxcCompiler3* pCompiler = res.get();
		
		// convert arguments to warguments (i hate this i hate this i hate this)
		vector<string> arguments = make_compile_args_strings(signature, args);
		vector<string> warguments{}; warguments.resize(arguments.size());
		vector<LPCWSTR> lwarguments{}; lwarguments.resize(arguments.size());
		for (uint64 i = 0u; i < arguments.size(); ++i)
		{
			warguments[i] = to_wstring(arguments[i]);
			lwarguments[i] = warguments[i].c_wstr();
		}

		influx_include_handler include_handler{};
		include_handler.set_include_folder(args.m_include_folder);

		// COMPILE
		IDxcResult* pCompileResult;
		hres = pCompiler->Compile(&buffer, lwarguments.data(),
			(uint32)lwarguments.size(), &include_handler, IID_PPV_ARGS(&pCompileResult));
		if (hres != S_OK)
		{
			return result_type::make_error("error: Compile failed");
		}

		// handle compile errors / warnings
		{
			string errors = get_compile_errors(*pCompileResult);
			std::wistringstream stream(errors.c_wstr());
			string line;

			// parse the whole log
			bool has_true_error = false;
			bool has_warning = false;
			while (std::getline( stream, line.get_wstd() ))
			{
				const bool is_empty = line.empty();
				if (is_empty) continue;

				const bool is_warning = line.contains("warning", false);
				const bool is_error = line.contains("error", false);

				has_warning |= is_warning;
				has_true_error |= is_error;

				result.get().m_log.push_back(line);
				wprintf( line.c_wstr() ); printf("\n");
			}

			if (has_true_error)
			{
				result.get().m_success = false;
				result.get_unex() = "error: compile result contains errors!";
				return result;
			}
			else if (has_warning)
			{
				result = result_type::make_warning({}, "compile warnings (see log)!");
			}
		}

		// [OUTPUT: SHADER BYTECODE]
		{
			auto res = get_compile_bytecode(*pCompileResult);
			if (res.is_success())
			{
				result.get().m_bytecode = res.get();
				result.get().m_success = true;
			}
			else return result_type::make_error("error: get_compile_bytecode() failed!");
		}

		// [OUTPUT: DEBUG INFO]
		if (args.m_pbd_enabled && !args.m_pdb_folder.empty())
		{
			// ensure the directory exists
			if (!influx::path::is_directory(args.m_pdb_folder))
			{
				influx::path::create_directory(args.m_pdb_folder);
			}

			IDxcBlob* pDebugData = nullptr;
			IDxcBlobUtf16* pDebugDataPath = nullptr;
			hres = pCompileResult->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pDebugData), &pDebugDataPath);
			if (hres == S_OK && pDebugData != nullptr)
			{
				string foldername = args.m_pdb_folder;
				string filename = args.m_pdb_filename + make_shader_name_string(signature, args);
				string filepath = foldername + L"/" + filename + L".pdb";

				hres = WriteBlobToFile((ID3DBlob*)pDebugData,
					filepath.c_wstr());
			}
			else
			{
				return result_type::make_error("error: GetOutput(DXC_OUT_PDB) failed!");
			}
		}

		// [OUTPUT: ROOT SIGNATURE]
		if (false)
		{
			IDxcBlob* pRootSignature = nullptr;
			IDxcBlobUtf16* pRootSignatureDataPath = nullptr;
			hres = pCompileResult->GetOutput(DXC_OUT_ROOT_SIGNATURE, IID_PPV_ARGS(&pRootSignature), &pRootSignatureDataPath);
			if (hres == S_OK && pRootSignature)
			{
				// ... todo
			}
			else
			{
				return result_type::make_error("error: GetOutput(DXC_OUT_ROOT_SIGNATURE) failed!");
			}
		}
		
		// [OUTPUT: REFLECTION DATA]
		if (args.m_reflection_enabled)
		{
			IDxcBlob* pReflectionData = nullptr;
			ID3D12ShaderReflection* pShaderReflection = nullptr;
			hres = pCompileResult->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&pReflectionData), nullptr);
			if (hres == S_OK && pReflectionData)
			{
				DxcBuffer reflectionBuffer;
				reflectionBuffer.Ptr = pReflectionData->GetBufferPointer();
				reflectionBuffer.Size = pReflectionData->GetBufferSize();
				reflectionBuffer.Encoding = 0;

				hres = get_utils()->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&pShaderReflection));
				result.get().m_reflection = reflect_shader(pShaderReflection);
			}
			else
			{
				return result_type::make_error("error: GetOutput(DXC_OUT_REFLECTION) failed!");
			}
		}

		// ULTIMATE SUCCESS
		result.get().m_signature = signature;
		result.get().m_signature.m_target = args.m_target;
		result.get().m_signature.cache_id();
		result.get().m_success = true;
		return result;
	}

	result<DxcBuffer> filepath_to_buffer(const string& filepath)
	{
		using result_type = result<DxcBuffer>;

		if (filepath.empty()) return result_type::make_error("error: empty filepath!");
		if (!path::exists(filepath)) return result_type::make_error("error: non-exist filepath!");

		// load the file
		string wfilepath = filepath;
		IDxcBlobEncoding* pShaderSourceFile;
		HRESULT
		hresult = get_utils()->LoadFile(wfilepath.c_wstr(), nullptr, &pShaderSourceFile);
		if (hresult != S_OK)
		{
			return result_type::make_error("error: IDxcUtils->LoadFile failed!");
		}

		DxcBuffer sourceBuffer;
		sourceBuffer.Ptr = pShaderSourceFile->GetBufferPointer();
		sourceBuffer.Size = pShaderSourceFile->GetBufferSize();
		sourceBuffer.Encoding = 0u;
		return sourceBuffer;
	}

	result<compile_output> compile_shader_in_file(
		const string& filepath, 
		const shader_signature& signature,
		const compile_args& args)
	{
		using result_type = result<compile_output>;

		if (!signature.is_valid()) 
			return result_type::make_error("error: signature is invalid!");

		auto res = filepath_to_buffer(filepath);
		if (res.is_fail())
			return result_type::make_error("error: failed converting file to dxc buffer!");

		return compile_shader_dxcbuffer(res.get(), signature, args);
	}

	result<compile_output> compile_shader_in_source(
		const string& shader_source, 
		const shader_signature& signature,
		const compile_args& args)
	{
		using result_type = result<compile_output>;

		if (!signature.is_valid())
			return result_type::make_error("error: signature is invalid!");

		if (shader_source.empty())
			return result_type::make_error("error: emtpy shader source!");

		std_str source_std = shader_source.get_std();
		DxcBuffer sourceBuffer;
		sourceBuffer.Ptr = source_std.c_str();
		sourceBuffer.Size = source_std.size();
		sourceBuffer.Encoding = 0u; // ANSI

		return compile_shader_dxcbuffer(sourceBuffer, signature, args);
	}

	result<shader_library> compile_shader_library(const string& filepath, const shader_library_compile_args& args)
	{
		using result_type = result<shader_library>;

		if (filepath.empty())
			return result_type::make_error("error: invalid filepath!");

		// filepath -> buffer
		DxcBuffer buffer{};
		{
			auto res = filepath_to_buffer(filepath);
			if (res.is_fail())
				return result_type::make_error("error: failed converting filepath to DxcBuffer!");
			buffer = res.get();
		}	
		
		// make the compiler
		IDxcCompiler3* compiler = nullptr;
		{
			auto res = create_compiler();
			if (res.is_fail())
				return result_type::make_error("error: DxcCreateInstance failed");
			compiler = res.get();
		}

		// setup the arguments
		vector<LPCWSTR> warguments{};
		const string target = build_shaderlib_target_string(args.m_target);
		const string wtarget = target;
		warguments.push_back(L"-T ");
		warguments.push_back(wtarget.c_wstr());

		const string wentrypoint = args.m_entrypoint;
		warguments.push_back(L"-E ");
		warguments.push_back(wentrypoint.c_wstr());

		// compile!
		IDxcResult* compile_result = nullptr;
		HRESULT 
		hres = compiler->Compile(&buffer, warguments.data(), (uint32)warguments.size(), nullptr, IID_PPV_ARGS(&compile_result));
		if (hres != S_OK)
			return result_type::make_error("error: Compile failed!");

		result_type result{};

		// compile errors:
		{
			// if have errors, add to result
			const string& errors = get_compile_errors(*compile_result);
			if (errors.empty() == false)
			{
				result.get().m_log.push_back(errors);
				result.get().m_success = false;
				result.get_unex() = "error: compile failed with errors!";
				return result;
			}
		}

		// compile bytecode:
		{
			auto res = get_compile_bytecode(*compile_result);
			if (res.is_fail())
				return result_type::make_error("error: failed parsing compiled bytecode!");
			
			result.get().m_bytecode = res.get();
		}

		return result;
	}
}
#endif // INFLUX_SHADER_BACKEND_DXC