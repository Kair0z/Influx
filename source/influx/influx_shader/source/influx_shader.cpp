#include "influx_shader.h"

#include "core/file.h"
#include "core/container/map.h"
#include "core/regex.h"

// https://strontic.github.io/xcyclopedia/library/dxc.exe-0C1709D4E1787E3EB3E6A35C85714824.html

// dx12 compiler
#include <d3dcompiler.h>
#include <dxcapi.h>
#pragma comment (lib, "d3dcompiler.lib")
#pragma comment (lib, "dxcompiler.lib")

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
			string filename = file(to_string(pFilename)).m_filename;
			
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
				auto rooted_path = to_wstring(m_include_folder) + pFilename;
				hr = get_utils()->LoadFile(rooted_path.c_str(), nullptr, &pEncoding);
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

	inline static string build_shader_target_string(e_shader_type type, e_shader_target target)
	{
		string result{};

		if (is_raytracing_shader(type))
		{
			// raytracing shaders are compiled as libs!!
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

	inline uint32 calc_num_floats_from_mask(uint32 mask)
	{
		uint32 num_floats = 0u;
		while (mask) {
			num_floats += mask & 1;
			mask >>= 1;
		}
		return num_floats;
	}

	inline reflection reflect_shader(ID3D12ShaderReflection* dx12_refl)
	{
		reflection result{};
		
		HRESULT hres = {};
		D3D12_SHADER_DESC shader_desc{};
		hres = dx12_refl->GetDesc(&shader_desc);

		// get input parameters
		for (uint32 i = 0u; i < shader_desc.InputParameters; ++i)
		{
			D3D12_SIGNATURE_PARAMETER_DESC signatureParameterDesc{};
			hres = dx12_refl->GetInputParameterDesc(i, &signatureParameterDesc);

			reflection::input_param param{};
			param.m_semantic_name = signatureParameterDesc.SemanticName;
			param.m_semantic_index = signatureParameterDesc.SemanticIndex;
			param.m_num_floats = calc_num_floats_from_mask(signatureParameterDesc.Mask);
			result.m_input_params.push_back(param);
		}

		// get bound resources
		for (uint32 i = 0u; i < shader_desc.BoundResources; ++i)
		{
			reflection::resource resource{};

			D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc{};
			hres = dx12_refl->GetResourceBindingDesc(i, &shaderInputBindDesc);
			resource.m_shader_register = shaderInputBindDesc.BindPoint;
			resource.m_register_space = shaderInputBindDesc.Space;
			resource.m_range_size = shaderInputBindDesc.BindCount;
			resource.m_name = string(shaderInputBindDesc.Name);

			// get variable info
			D3D12_SHADER_VARIABLE_DESC variable_desc{};
			ID3D12ShaderReflectionVariable* variable_refl = dx12_refl->GetVariableByName(shaderInputBindDesc.Name);
			hres = variable_refl->GetDesc(&variable_desc);
			
			switch (shaderInputBindDesc.Type)
			{
			case D3D_SHADER_INPUT_TYPE::D3D_SIT_STRUCTURED:
			{
				resource.m_type = reflection::resource::e_type::structured;

			}
			break;

			case D3D_SHADER_INPUT_TYPE::D3D_SIT_CBUFFER:
			{
				resource.m_type = reflection::resource::e_type::cbv;

				D3D12_SHADER_BUFFER_DESC constantBufferDesc{};
				ID3D12ShaderReflectionConstantBuffer* cbuffer_refl = dx12_refl->GetConstantBufferByIndex(i);
				hres = cbuffer_refl->GetDesc(&constantBufferDesc);

				resource.m_bytesize = constantBufferDesc.Size;
				// ...
			}
			break;

			case D3D_SHADER_INPUT_TYPE::D3D_SIT_SAMPLER:
			{
				resource.m_type = reflection::resource::e_type::sampler;
				// ...
			}
			break;

			case D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE:
			{
				resource.m_type = reflection::resource::e_type::texture;
				// ...
			}
			break;

			default:
				resource.m_type = reflection::resource::e_type::unknown;
				// ...
				break;
			}

			result.m_bound_resources.push_back(resource);
		}

		return result;
	}

	inline static wstring make_shader_name_string(const compile_args& args)
	{
		wstring type = to_wstring(build_shader_target_string(args.m_signature.m_type, args.m_signature.m_target));
		wstring entry = to_wstring(args.m_signature.m_entrypoint);
		return type + entry;
	}

	inline vector<string> build_dxc_compile_args_strings(const compile_args& args)
	{
		vector<string> result{};

		// entrypoint (-E)
		const string& entrypoint = args.m_signature.m_entrypoint;
		result.push_back("-E ");
		result.push_back(entrypoint.c_str());

		// exports (-exports)
		// (in case of raytracing, we compile a single shader as a shader lib and so declare a single export)
		const bool compile_as_shaderlib = shader::is_raytracing_shader(args.m_signature.m_type);
		if (compile_as_shaderlib)
		{
			result.push_back("-exports ");
			result.push_back(entrypoint.c_str());
		}

		// target (-T) (eg. ps_6_2)
		result.push_back("-T ");
		string profile = build_shader_target_string(args.m_signature.m_type, args.m_signature.m_target);
		result.push_back(profile.c_str());

		// includes (-I)
		result.push_back("-I ");
		const string& include_folder = args.m_include_folder;
		result.push_back(include_folder.c_str());

		// defines (-D)
		for (const string& define : args.m_defines)
		{
			result.push_back("-D");
			result.push_back(define.c_str());
		}

		// misc
		const bool row_major = false;
		result.push_back("dxc -help | findstr Version");
		result.push_back(row_major ? "-Zp" : "Zpc");
		if (!args.m_pbd) result.push_back("-Qstrip_debug");
		if (!args.m_reflection) result.push_back("-Qstrip_reflect");
		if (args.m_compile_debug) result.push_back("-Od"); // DXC_ARG_SKIP_OPTIMIZATIONS
		if (args.m_compile_debug) result.push_back("-O0"); // DXC_ARG_OPTIMIZATION_LEVEL0
		if (args.m_compile_debug) result.push_back("-Zi"); // DXC_ARG_DEBUG

		return result;
	}

	inline compile_output compile_shader_dxcbuffer(const DxcBuffer& buffer, const compile_args& args)
	{
		HRESULT result{};
		compile_output output = {};

		// https://simoncoenen.com/blog/programming/graphics/DxcCompiling
		// create the Dxc Compiler
		IDxcCompiler3* pCompiler;
		result = ::DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));

		// convert arguments to warguments
		vector<string> arguments = build_dxc_compile_args_strings(args);
		vector<LPCWSTR> warguments{};
		warguments.reserve(arguments.size());
		for (const string& arg : arguments)
		{
			warguments.push_back(to_wstring(arg).c_str());
		}

		//
		influx_include_handler include_handler{};
		include_handler.set_include_folder(args.m_include_folder);

		// COMPILE
		IDxcResult* pCompileResult;
		result = pCompiler->Compile(&buffer, warguments.data(),
			(uint32)warguments.size(), &include_handler, IID_PPV_ARGS(&pCompileResult));

		// [OUTPUT: COMPILE ERRORS]
		IDxcBlobUtf8* pErrors = nullptr;
		result = pCompileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
		if (pErrors && pErrors->GetStringLength() > 0)
		{
			output.m_log.push_back(string((char*)pErrors->GetBufferPointer()));
			printf(output.m_log.back().c_str());
			printf("\n");
			output.m_success = false;
			return output;
		}

		// [OUTPUT: DEBUG INFO]
		if (args.m_pbd && !args.m_pdb_folder.empty())
		{
			if (!influx::file::is_directory(args.m_pdb_folder))
			{
				influx::file::make_directory(args.m_pdb_folder);
			}

			IDxcBlob* pDebugData = nullptr;
			IDxcBlobUtf16* pDebugDataPath = nullptr;
			result = pCompileResult->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pDebugData), &pDebugDataPath);
			if (result == S_OK && pDebugData != nullptr)
			{
				wstring foldername = to_wstring(args.m_pdb_folder);
				wstring filename = to_wstring(args.m_pdb_filename) + make_shader_name_string(args);
				wstring filepath = foldername + L"/" + filename + L".pdb";

				result = ::D3DWriteBlobToFile((ID3DBlob*)pDebugData, 
					filepath.c_str(), true);
			}
		}

		// [OUTPUT: ROOT SIGNATURE]
		if (false)
		{
			IDxcBlob* pRootSignature = nullptr;
			IDxcBlobUtf16* pRootSignatureDataPath = nullptr;
			result = pCompileResult->GetOutput(DXC_OUT_ROOT_SIGNATURE, IID_PPV_ARGS(&pRootSignature), &pRootSignatureDataPath);
			if (pRootSignature)
			{

			}
		}
		
		// [OUTPUT: REFLECTION DATA]
		if (args.m_reflection)
		{
			IDxcBlob* pReflectionData = nullptr;
			ID3D12ShaderReflection* pShaderReflection = nullptr;
			result = pCompileResult->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&pReflectionData), nullptr);
			if (pReflectionData)
			{
				DxcBuffer reflectionBuffer;
				reflectionBuffer.Ptr = pReflectionData->GetBufferPointer();
				reflectionBuffer.Size = pReflectionData->GetBufferSize();
				reflectionBuffer.Encoding = 0;

				result = get_utils()->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&pShaderReflection));
				output.m_reflection = reflect_shader(pShaderReflection);
			}
		}

		// [OUTPUT: RESULT SHADER BYTE CODE]
		IDxcBlob* pResultData = nullptr;
		IDxcBlobUtf16* pResultOutputName = nullptr;
		result = pCompileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pResultData), &pResultOutputName);
		if (pResultData)
		{
			for (uint32 i = 0; i < pResultData->GetBufferSize(); ++i)
			{
				output.m_bytecode.push_back(reinterpret_cast<byte*>(pResultData->GetBufferPointer())[i]);
			}
		}

		output.m_signature = args.m_signature;
		output.m_signature.cache_id();
		output.m_success = true;
		return output;
	}

	result<compile_output> compile_shader_in_file(const string& filepath, const compile_args& args)
	{
		using result_type = result<compile_output>;

		if (filepath.empty()) return result_type::make_error("error: empty filepath!");
		if (file::exists(filepath)) return result_type::make_error("error: non-exist filepath!");
		if (args.is_valid()) return result_type::make_error("error: compile args are invalid!");

		// load the file
		HRESULT hresult{};
		wstring wfilepath = to_wstring(filepath);
		IDxcBlobEncoding* pShaderSourceFile;
		hresult = get_utils()->LoadFile(wfilepath.c_str(), nullptr, &pShaderSourceFile);

		DxcBuffer sourceBuffer;
		sourceBuffer.Ptr = pShaderSourceFile->GetBufferPointer();
		sourceBuffer.Size = pShaderSourceFile->GetBufferSize();
		sourceBuffer.Encoding = 0u;

		return compile_shader_dxcbuffer(sourceBuffer, args);
	}

	result<compile_output> compile_shader(const string& shader_source, const compile_args& args)
	{
		if (shader_source.empty())
		{
			return result<compile_output>::make_error("error: emtpy shader source!");
		}
		DxcBuffer sourceBuffer;
		sourceBuffer.Ptr = shader_source.c_str();
		sourceBuffer.Size = shader_source.size();
		sourceBuffer.Encoding = 0u; // ANSI

		return compile_shader_dxcbuffer(sourceBuffer, args);
	}

	// parse out potential shader types
	static const char* type_to_signature[] =
	{
		R"(\[shader\(\"vertex\"\)\])",
		R"(\[shader\(\"pixel\"\)\])",
		R"(\[shader\(\"domain\"\)\])",
		R"(\[shader\(\"geometry\"\)\])",
		R"(\[shader\(\"hull\"\)\])",

		R"(\[shader\(\"compute\"\)\])",

		R"(\[shader\(\"raygeneration\"\)\])",
		R"(\[shader\(\"miss\"\)\])",
		R"(\[shader\(\"closesthit\"\)\])",
		R"(\[shader\(\"anyhit\"\)\])",
		R"(\[shader\(\"intersection\"\)\])",

		R"(\[shader\(\"amp\"\)\])",
		R"(\[shader\(\"mesh\"\)\])",
	};
	// see core::shader
	static_assert(_countof(type_to_signature) == shader::k_num_shadertypes);

	result<vector<parse_output>> parse_shaders_in_file(const string& filepath)
	{
		using result_type = result<vector<parse_output>>;
		result_type result{};

		if (file::exists(filepath) == false)
		{
			return result_type::make_error("error: filepath doesnt exist!");
		}

		string file_content = file::content_to_string(filepath);
		if (file_content.empty())
		{
			return result_type::make_error("error: file is empty!");
		}

		return parse_shader(file_content);
	}

	result<vector<parse_output>> parse_shader(const string& shader_source)
	{
		using result_type = result<vector<parse_output>>;
		if (shader_source.empty())
		{
			return result_type::make_error("error: source string is empty!");
		}

		result_type result{};
		vector<string> source_lines = str::split(shader_source, "\n");
		for (uint32 i = 0u; i < shader::k_num_shadertypes; ++i)
		{
			for (uint32 l = 0u; l < source_lines.size(); ++l)
			{
				const string& line = source_lines[l];

				// search each line for the current type's signature ([shader("vertex")])
				influx::regex::for_each_match(line, type_to_signature[i],
				[i, &source_lines, l, &result](const string& str)
				{
					// now figure out the function entrypoint name:
					// todo: make this a bit more error-proof
					uint32 next_idx = l + 1;
					string next_line = source_lines[next_idx];
					static uint32 max_it = l + 100;
					while ((next_line.empty() || next_line[0] == '[') && next_idx < max_it) next_line = source_lines[next_idx++];

					// found the entrypoint line, parse the entrypoint
					vector<string> entrypoints = regex::get_all_matches(next_line, R"(\b\w+\s+(\w+)\()");
					if (entrypoints.size() > 0 && entrypoints[0].empty() == false)
					{
						const string& entrypoint = entrypoints[0];
						const e_shader_type current_shader_type = static_cast<shader::e_shader_type>(i);

						parse_output new_shader_parse{};
						new_shader_parse.m_type = current_shader_type;
						new_shader_parse.m_entrypoint = entrypoint;
						new_shader_parse.m_compile_args.m_signature.m_type = current_shader_type;
						new_shader_parse.m_compile_args.m_signature.m_entrypoint = entrypoint;
						result.get().push_back(new_shader_parse);
					}
				});
			}
		}

		return result;
	}
}