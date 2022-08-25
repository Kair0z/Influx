#include "pch.h"
#include "Shader.h"

#if PLATFORM_WINDOWS
#define FLX_SHADERCOMPILER_D3DCOMPILER
#define FLX_SHADERCOMPILER_DXCOMPILER // Allows Spirv
#endif

#ifdef FLX_SHADERCOMPILER_D3DCOMPILER
#include <d3dcompiler.h>
#endif

#ifdef FLX_SHADERCOMPILER_DXCOMPILER
#include "DXC/dxcapi.h"
#pragma comment (lib, "dxcompiler.lib")
#endif

namespace Influx::Shaders
{
	Shader* Shader::Compile(const String& filepath, const String& entryPoint, const EShaderType type, const EShaderModel model,
		const Vector<String>& includeDirectories, const Vector<String>& defines)
	{
		Shader* newShader = new Shader();
		newShader->mCompileFilepath = filepath;
		newShader->mFlags;
		newShader->mShaderModel = model;
		newShader->mShaderType = type;
		
		String constructedShaderString = MakeShaderString(type, model);

#ifdef FLX_SHADERCOMPILER_D3DCOMPILER
		// Todo... If invalid shader model...

		D3D_SHADER_MACRO d3dDef[] = {
			"HLSL5", "1", "DISABLE_WAVE_INTRINSICS", "1", NULL, NULL
		};

		// Todo... Includes

		// Todo... flags

		ID3DBlob* pBytecode;
		ID3DBlob* pErrors;
		HRESULT hr = D3DCompileFromFile(ToWString(filepath).c_str(), d3dDef, nullptr, entryPoint.c_str(), constructedShaderString.c_str(), 0, 0, &pBytecode, &pErrors);
		if (pErrors)
		{
			newShader->mCompileOutput.ErrorMessage = (const char*)pErrors->GetBufferPointer();
		}

		if (SUCCEEDED(hr))
		{
			newShader->mCompileOutput.pShaderData = (const uint8_t*)pBytecode->GetBufferPointer();
			newShader->mCompileOutput.ShaderSize = pBytecode->GetBufferSize();
		}
		else
		{
			assert(false);
		}
#endif

#ifdef FLX_SHADERCOMPILER_DXCOMPILER
		Vector<LPWSTR> arguments;
		arguments.push_back(L"-E");
		// Todo...
#endif
	}

	bool Shader::ParseShaderString(const String& string, EShaderType& outType, EShaderModel& outModel)
	{
		size_t first_ = string.find_first_of("_");

		const String& shaderTypeSub = string.substr(0, first_);
		if (shaderTypeSub == "vs")
		{
			outType = EShaderType::VertexShader;
		}
		else if (shaderTypeSub == "ps")
		{
			outType = EShaderType::PixelShader;
		}
		else
		{
			return false;
		}

		const std::string& modelSub = string.substr(first_ + 1);
		if (modelSub == "5_0")
		{
			outModel = EShaderModel::SM_5_0;
		}
		else
		{
			return false;
		}

		return true;
	}

	String Shader::MakeShaderString(const EShaderType type, const EShaderModel model)
	{
		String resultString{};

		switch (type)
		{
		case EShaderType::PixelShader:
			resultString.append("ps_");
			break;

		case EShaderType::VertexShader:
			resultString.append("vs_");
			break;

		default:
			break;
		}

		switch (model)
		{
		case EShaderModel::SM_5_0:
			resultString.append("5_0");
			break;

		default:
			break;
		}

		return resultString;
	}

	bool ShaderCompileOutput::IsValid() const
	{
		return pShaderData != nullptr;
	}
}