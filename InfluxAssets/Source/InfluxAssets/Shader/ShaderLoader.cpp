#include "InfluxAssets/InfluxAssets.h"

#include <d3dcompiler.h>
#include <dxcapi.h>

#pragma comment (lib, "dxcompiler.lib")

namespace Influx::Assets
{
	/* Loads a Shader file (.hlsl) */
	bool LoadShaderFile(const String& filepath, ShaderData& out_shaderData, ShaderCachePtr pCache, const ShaderLoadDesc& loadDesc)
	{
		// Try to find the loaded scene in the provided cache...
		if (pCache)
		{
			if (pCache->Contains(filepath, loadDesc))
			{
				// Copy!
				out_shaderData = *pCache->Get(filepath, loadDesc);
				return true;
			}
		}


		// Compile settings:
		constexpr bool bStripPBD = false;
		constexpr bool bStripReflection = false;
		constexpr bool bStripDebug = false;
		Vector<WString> defines{};

		// [WORK] Create new data...
		ShaderData shaderData;
		{
			Vector<byte> outResult{};

			// https://simoncoenen.com/blog/programming/graphics/DxcCompiling
			HRESULT result;

			IDxcUtils* pUtils;
			result = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));

			IDxcBlobEncoding* pShaderSourceFile;
			result = pUtils->LoadFile(L"", nullptr, &pShaderSourceFile);

			IDxcCompiler3* pCompiler;
			result = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));

			Vector<LPCWSTR> arguments;
			//-E for the entry point (eg. VSMain)
			arguments.push_back(L"-E");
			arguments.push_back(L"");

			//-T for the target profile (eg. ps_6_2)
			arguments.push_back(L"-T");

			WString shaderModelString{};
#if 0
			switch (desc.Type)
			{
			case ShaderDesc::EType::PS:
				shaderModelString.append(L"ps");
				break;

			case ShaderDesc::EType::VS:
				shaderModelString.append(L"vs");
				break;

			default:
				assert(false);
				break;
			}
			switch (desc.Profile)
			{
			case ShaderDesc::EProfile::_6_2:
				shaderModelString.append(L"_6_2");
				break;

			default:
				assert(false);
				break;
			}
#endif

			arguments.push_back(shaderModelString.c_str());

			arguments.push_back(L"dxc -help | findstr Version");

			// Strip reflection data and pdbs (see later)
			// "The compiler will strip both the shader PDBs and reflection data from the Object part"
			// "it will STILL be in the compile result and can be extracted using DXC_OUT_PDB and DXC_OUT_REFLECTION"!!
			if (bStripPBD) arguments.push_back(L"-Qstrip_debug");
			if (bStripReflection) arguments.push_back(L"-Qstrip_reflect");
			// arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS); //-WX
			if (!bStripDebug) arguments.push_back(DXC_ARG_DEBUG); //-Zi
			// arguments.push_back(DXC_ARG_SKIP_VALIDATION);
			arguments.push_back(DXC_ARG_PACK_MATRIX_ROW_MAJOR); //-Zp

			for (const WString& define : defines)
			{
				arguments.push_back(L"-D");
				arguments.push_back(define.c_str());
			}

			DxcBuffer sourceBuffer;
			sourceBuffer.Ptr = pShaderSourceFile->GetBufferPointer();
			sourceBuffer.Size = pShaderSourceFile->GetBufferSize();
			sourceBuffer.Encoding = 0;

			IDxcResult* pCompileResult;
			result = pCompiler->Compile(&sourceBuffer, arguments.data(), (uint32)arguments.size(), nullptr, IID_PPV_ARGS(&pCompileResult));

			// [OUT COMPILE ERROR]
			IDxcBlobUtf8* pErrors = nullptr;
			result = pCompileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
			if (pErrors && pErrors->GetStringLength() > 0)
			{
				// Do something with... (char*)pErrors->GetBufferPointer()
			}

			// [OUT DEBUG INFO]
			IDxcBlob* pDebugData = nullptr;
			IDxcBlobUtf16* pDebugDataPath = nullptr;
			result = pCompileResult->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pDebugData), &pDebugDataPath);

			IDxcBlob* pRootSignature = nullptr;
			IDxcBlobUtf16* pRootSignatureDataPath = nullptr;
			result = pCompileResult->GetOutput(DXC_OUT_ROOT_SIGNATURE, IID_PPV_ARGS(&pRootSignature), &pRootSignatureDataPath);
			if (pRootSignature)
			{

			}

			// [OUT REFLECTION DATA]
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

			// [OUT SHADER BYTE CODE]
			IDxcBlob* pResultData = nullptr;
			IDxcBlobUtf16* pResultOutputName = nullptr;
			result = pCompileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pResultData), &pResultOutputName);
			if (pResultData)
			{
				for (uint32 i = 0; i < pResultData->GetBufferSize(); ++i)
				{
					outResult.push_back(reinterpret_cast<byte*>(pResultData->GetBufferPointer())[i]);
				}
			}
		}
		

		// Cache loaded scene result:
		if (pCache)
		{
			if (!pCache->Contains(filepath, loadDesc))
			{
				// Copy sceneData into scene-cache!
				pCache->Add(filepath, shaderData, loadDesc);
			}
		}

		out_shaderData = shaderData;
		return true;
	}
}