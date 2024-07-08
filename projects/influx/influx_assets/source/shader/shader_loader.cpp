#include "assets_pch.h"
#include "influx_assets.h"

#include "core/log.h"

// dx12 compiler
#include <d3dcompiler.h>
#include <dxcapi.h>
#pragma comment (lib, "dxcompiler.lib")

namespace influx::assets
{
	static wstring make_shader_type_string(e_shader_type type, e_shader_target target)
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
		}

		return result;
	}

	/* Loads a Shader file (.hlsl) */
	bool load_shader_file(const string& filepath, shader_data& out_shader, const shader_load_args& args)
	{
		out_shader.m_compile_result = {};

		// https://simoncoenen.com/blog/programming/graphics/DxcCompiling
		// create the Dxc Utils
		HRESULT result;
		IDxcUtils* pUtils;
		result = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));

		// load the file
		wstring wfilepath = to_wstring(filepath);
		IDxcBlobEncoding* pShaderSourceFile;
		result = pUtils->LoadFile(wfilepath.c_str(), nullptr, &pShaderSourceFile);

		// create the Dxc Compiler
		IDxcCompiler3* pCompiler;
		result = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));

		// Gather arguments
		vector<LPCWSTR> arguments;
		wstring wentrypoint = to_wstring(args.m_entrypoint);

		//-E for the entry point (eg. VSMain)
		arguments.push_back(L"-E");
		arguments.push_back(wentrypoint.c_str());

		//-T for the target profile (eg. ps_6_2)
		arguments.push_back(L"-T");

		wstring target_profile = make_shader_type_string(args.m_type, args.m_target);
		arguments.push_back(target_profile.c_str());
		arguments.push_back(L"dxc -help | findstr Version");

		// Strip reflection data and pdbs (see later)
		// "The compiler will strip both the shader PDBs and reflection data from the Object part"
		// "it will STILL be in the compile result and can be extracted using DXC_OUT_PDB and DXC_OUT_REFLECTION"!!
		if (!args.m_pbd) arguments.push_back(L"-Qstrip_debug");
		if (!args.m_reflection) arguments.push_back(L"-Qstrip_reflect");
		// arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS); //-WX
		if (args.m_compile_debug) arguments.push_back(DXC_ARG_DEBUG); //-Zi
		// arguments.push_back(DXC_ARG_SKIP_VALIDATION);
		arguments.push_back(DXC_ARG_PACK_MATRIX_ROW_MAJOR); //-Zp

		for (const string& define : args.m_defines)
		{
			arguments.push_back(L"-D");
			arguments.push_back(to_wstring(define).c_str());
		}

		DxcBuffer sourceBuffer;
		sourceBuffer.Ptr = pShaderSourceFile->GetBufferPointer();
		sourceBuffer.Size = pShaderSourceFile->GetBufferSize();
		sourceBuffer.Encoding = 0;

		// COMPILE
		IDxcResult* pCompileResult;
		result = pCompiler->Compile(&sourceBuffer, arguments.data(), 
			(uint32)arguments.size(), nullptr, IID_PPV_ARGS(&pCompileResult));

		// [OUTPUT: COMPILE ERRORS]
		IDxcBlobUtf8* pErrors = nullptr;
		result = pCompileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
		if (pErrors && pErrors->GetStringLength() > 0)
		{
			// Do something with... (char*)pErrors->GetBufferPointer()
			printf(((char*)pErrors->GetBufferPointer()));

			logerr("influx_assets::load_shader_file() failed!");
			return false;
		}

		// [OUTPUT: DEBUG INFO]
		IDxcBlob* pDebugData = nullptr;
		IDxcBlobUtf16* pDebugDataPath = nullptr;
		result = pCompileResult->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pDebugData), &pDebugDataPath);

		// [OUTPUT: ROOT SIGNATURE]
		IDxcBlob* pRootSignature = nullptr;
		IDxcBlobUtf16* pRootSignatureDataPath = nullptr;
		result = pCompileResult->GetOutput(DXC_OUT_ROOT_SIGNATURE, IID_PPV_ARGS(&pRootSignature), &pRootSignatureDataPath);
		if (pRootSignature)
		{

		}

		// [OUTPUT: REFLECTION DATA]
		IDxcBlob* pReflectionData = nullptr;
		ID3D12ShaderReflection* pShaderReflection = nullptr;
		result = pCompileResult->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&pReflectionData), nullptr);
		if (pReflectionData)
		{
			DxcBuffer reflectionBuffer;
			reflectionBuffer.Ptr = pReflectionData->GetBufferPointer();
			reflectionBuffer.Size = pReflectionData->GetBufferSize();
			reflectionBuffer.Encoding = 0;

			result = pUtils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&pShaderReflection));
			// ...
		}

		// [OUTPUT: RESULT SHADER BYTE CODE]
		IDxcBlob* pResultData = nullptr;
		IDxcBlobUtf16* pResultOutputName = nullptr;
		result = pCompileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pResultData), &pResultOutputName);
		if (pResultData)
		{
			for (uint32 i = 0; i < pResultData->GetBufferSize(); ++i)
			{
				out_shader.m_compile_result.push_back(reinterpret_cast<byte*>(pResultData->GetBufferPointer())[i]);
			}
		}

		return true;
	}
}