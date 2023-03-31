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

		struct CompileDesc final
		{
			enum class EShaderType : uint8
			{
				VertexShader,
				PixelShader,
				Max,
				Unsupported = Max
			} ShaderType;
			
			enum class EShaderTarget : uint8
			{
				_6_2,
				Max,
				Unsupported = Max
			} ShaderTarget;

			static WString GetShaderTypeString(EShaderType type, EShaderTarget target)
			{
				WString result{};

				switch (type)
				{
				case EShaderType::VertexShader: result += L"vs"; break;
				case EShaderType::PixelShader: result += L"ps"; break;
				case EShaderType::Unsupported: result += L"LLLL"; break;
				}

				result += L"_";

				switch (target)
				{
				case EShaderTarget::_6_2: result += L"6_2"; break;
				}

				return result;
			}

			WString GetShaderTypeString() const
			{
				return GetShaderTypeString(ShaderType, ShaderTarget);
			}

			WString Filepath = L"";
			WString Entrypoint = L"";

			Vector<WString> Defines{};

			bool bCompileDebug;
			bool bStripReflection;
			bool bStripPBD;
		};

		auto compile = [](const CompileDesc& desc) -> Vector<byte>
		{
			// Thanks to...
			// https://simoncoenen.com/blog/programming/graphics/DxcCompiling
			Vector<byte> outResult{};
			HRESULT result;

			IDxcUtils* pUtils;
			result = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));

			IDxcBlobEncoding* pShaderSourceFile;
			result = pUtils->LoadFile(desc.Filepath.c_str(), nullptr, &pShaderSourceFile);

			IDxcCompiler3* pCompiler;
			result = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));

			// Gather arguments
#pragma region gather arguments
			Vector<LPCWSTR> arguments;
			//-E for the entry point (eg. VSMain)
			arguments.push_back(L"-E");
			arguments.push_back(desc.Entrypoint.c_str());

			//-T for the target profile (eg. ps_6_2)
			arguments.push_back(L"-T");

			WString targetProfile = desc.GetShaderTypeString();
			arguments.push_back(targetProfile.c_str());
			arguments.push_back(L"dxc -help | findstr Version");

			// Strip reflection data and pdbs (see later)
			// "The compiler will strip both the shader PDBs and reflection data from the Object part"
			// "it will STILL be in the compile result and can be extracted using DXC_OUT_PDB and DXC_OUT_REFLECTION"!!
			if (desc.bStripPBD) arguments.push_back(L"-Qstrip_debug");
			if (desc.bStripReflection) arguments.push_back(L"-Qstrip_reflect");
			// arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS); //-WX
			if (desc.bCompileDebug) arguments.push_back(DXC_ARG_DEBUG); //-Zi
			// arguments.push_back(DXC_ARG_SKIP_VALIDATION);
			arguments.push_back(DXC_ARG_PACK_MATRIX_ROW_MAJOR); //-Zp

			for (const WString& define : desc.Defines)
			{
				arguments.push_back(L"-D");
				arguments.push_back(define.c_str());
			}
#pragma endregion

			DxcBuffer sourceBuffer;
			sourceBuffer.Ptr = pShaderSourceFile->GetBufferPointer();
			sourceBuffer.Size = pShaderSourceFile->GetBufferSize();
			sourceBuffer.Encoding = 0;

			// COMPILE
			IDxcResult* pCompileResult;
			result = pCompiler->Compile(&sourceBuffer, arguments.data(), (uint32)arguments.size(), nullptr, IID_PPV_ARGS(&pCompileResult));

			// [OUTPUT: COMPILE ERRORS]
			IDxcBlobUtf8* pErrors = nullptr;
			result = pCompileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
			if (pErrors && pErrors->GetStringLength() > 0)
			{
				// Do something with... (char*)pErrors->GetBufferPointer()
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
					outResult.push_back(reinterpret_cast<byte*>(pResultData->GetBufferPointer())[i]);
				}
			}

			return outResult;
		};
		
		ShaderData shaderData{};

		const bool compileDebug = true;
		const bool stripPBD = true;
		const bool stripReflection = true;
		const CompileDesc::EShaderTarget shaderTarget = CompileDesc::EShaderTarget::_6_2;
		
		CompileDesc compileDesc{};
		compileDesc.bCompileDebug = compileDebug;
		compileDesc.bStripPBD = stripPBD;
		compileDesc.bStripReflection = stripReflection;
		compileDesc.Defines;
		compileDesc.Filepath = ToWString(filepath);
		compileDesc.ShaderTarget = shaderTarget;

		// Pixel Shader
		compileDesc.Entrypoint = L"PSMain";
		compileDesc.ShaderType = CompileDesc::EShaderType::PixelShader;
		shaderData.PixelShader = compile(compileDesc);

		// Vertex Shader
		compileDesc.Entrypoint = L"VSMain";
		compileDesc.ShaderType = CompileDesc::EShaderType::VertexShader;
		shaderData.VertexShader = compile(compileDesc);

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