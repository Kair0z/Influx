#include "influx_shader.h"

#include "core/file.h"
#include "core/container/map.h"

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
			return hr;
		}

		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override { return E_NOINTERFACE; }
		ULONG STDMETHODCALLTYPE AddRef(void) override { return 0; }
		ULONG STDMETHODCALLTYPE Release(void) override { return 0; }

	private:
		umap<string, string> m_included_map{};
	};

	inline static wstring make_shader_type_wstring(e_shader_type type, e_shader_target target)
	{
		wstring result{};

		switch (type)
		{
		case e_shader_type::vs: result += L"vs"; break;
		case e_shader_type::ps: result += L"ps"; break;
		case e_shader_type::count: result += L"LLLL"; break;
		}

		result += L"_";

		switch (target)
		{
		case e_shader_target::_6_2: result += L"6_2"; break;
		case e_shader_target::_6_5: result += L"6_5"; break;
		case e_shader_target::_6_6: result += L"6_6"; break;
		}

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
		wstring type = make_shader_type_wstring(args.m_type, args.m_target);
		wstring entry = to_wstring(args.m_entrypoint);

		return type + entry;
	}

	inline compile_output compile_shader_dxcbuffer(const DxcBuffer& buffer, const compile_args& args)
	{
		HRESULT result{};
		compile_output output = {};

		// https://simoncoenen.com/blog/programming/graphics/DxcCompiling
		// create the Dxc Compiler
		IDxcCompiler3* pCompiler;
		result = ::DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));

		// Gather arguments
		vector<LPCWSTR> arguments;
		wstring wentrypoint = to_wstring(args.m_entrypoint);

		//-E for the entry point (eg. VSMain)
		arguments.push_back(L"-E");
		arguments.push_back(wentrypoint.c_str());

		//-T for the target profile (eg. ps_6_2)
		arguments.push_back(L"-T");
		wstring target_profile = make_shader_type_wstring(args.m_type, args.m_target);
		arguments.push_back(target_profile.c_str());

		arguments.push_back(L"dxc -help | findstr Version");

		// add include folder
		arguments.push_back(L"-I D:/Git/Influx/Resources/Shaders/include/");

		// Strip reflection data and pdbs (see later)
		// "The compiler will strip both the shader PDBs and reflection data from the Object part"
		// "it will STILL be in the compile result and can be extracted using DXC_OUT_PDB and DXC_OUT_REFLECTION"!!
		if (!args.m_pbd) arguments.push_back(L"-Qstrip_debug");
		if (!args.m_reflection) arguments.push_back(L"-Qstrip_reflect");
		// arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS); //-WX
		if (args.m_compile_debug) arguments.push_back(L"-Od");
		if (args.m_compile_debug) arguments.push_back(L"-O0");
		if (args.m_compile_debug) arguments.push_back(DXC_ARG_DEBUG); //-Zi
		// arguments.push_back(DXC_ARG_SKIP_VALIDATION);
		//arguments.push_back(DXC_ARG_PACK_MATRIX_ROW_MAJOR); //-Zp
		arguments.push_back(DXC_ARG_PACK_MATRIX_COLUMN_MAJOR); //-Zpc

		for (const string& define : args.m_defines)
		{
			arguments.push_back(L"-D");
			arguments.push_back(to_wstring(define).c_str());
		}
		
		influx_include_handler include_handler{};

		// COMPILE
		IDxcResult* pCompileResult;
		result = pCompiler->Compile(&buffer, arguments.data(),
			(uint32)arguments.size(), &include_handler, IID_PPV_ARGS(&pCompileResult));

		// [OUTPUT: COMPILE ERRORS]
		IDxcBlobUtf8* pErrors = nullptr;
		result = pCompileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
		if (pErrors && pErrors->GetStringLength() > 0)
		{
			printf(((char*)pErrors->GetBufferPointer()));
			printf("\n");
			influx_assert(false);

			// logerr("influx_assets::load_shader_file() failed!");
			return output;
		}

		// [OUTPUT: DEBUG INFO]
		if (args.m_pbd && !args.m_pdb_folder.empty())
		{
			influx_assert(influx::file::is_directory(args.m_pdb_folder));

			IDxcBlob* pDebugData = nullptr;
			IDxcBlobUtf16* pDebugDataPath = nullptr;
			result = pCompileResult->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pDebugData), &pDebugDataPath);
			if (result == S_OK && pDebugData != nullptr)
			{
				wstring foldername = to_wstring(args.m_pdb_folder);
				wstring filename = make_shader_name_string(args);
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

		return output;
	}

	compile_output compile_shader(const string& filepath, const compile_args& args)
	{
		HRESULT result{};
		
		// load the file
		wstring wfilepath = to_wstring(filepath);
		IDxcBlobEncoding* pShaderSourceFile;
		result = get_utils()->LoadFile(wfilepath.c_str(), nullptr, &pShaderSourceFile);

		DxcBuffer sourceBuffer;
		sourceBuffer.Ptr = pShaderSourceFile->GetBufferPointer();
		sourceBuffer.Size = pShaderSourceFile->GetBufferSize();
		sourceBuffer.Encoding = 0u;

		return compile_shader_dxcbuffer(sourceBuffer, args);
	}

	compile_output compile_shader_source(const string& shader_source, const compile_args& args)
	{
		DxcBuffer sourceBuffer;
		sourceBuffer.Ptr = shader_source.c_str();
		sourceBuffer.Size = shader_source.size();
		sourceBuffer.Encoding = 0u; // ANSI

		return compile_shader_dxcbuffer(sourceBuffer, args);
	}
}