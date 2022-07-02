#include "GraphicsAPI.h"

namespace Influx::Graphics
{
	std::string GraphicsAPI::MakeShaderTargetString(const ERHIShaderType shaderType, const ERHIShaderModel shaderModel)
	{
		std::string resultString{};

		switch (shaderType)
		{
		case ERHIShaderType::PixelShader:
			resultString.append("ps_");
			break;

		case ERHIShaderType::VertexShader:
			resultString.append("vs_");
			break;

		default:
			break;
		}

		switch (shaderModel)
		{
		case ERHIShaderModel::SM_5_0:
			resultString.append("5_0");
			break;

		default:
			break;
		}

		return resultString;
	}

	bool GraphicsAPI::ParseShaderTargetString(const std::string& targetString, ERHIShaderType& outShaderType, ERHIShaderModel& outShaderModel)
	{
		size_t first_ = targetString.find_first_of("_");

		const std::string& shaderTypeSub = targetString.substr(0, first_);
		if (shaderTypeSub == "vs")
		{
			outShaderType = ERHIShaderType::VertexShader;
		}
		else if (shaderTypeSub == "ps")
		{
			outShaderType = ERHIShaderType::PixelShader;
		}
		else
		{
			return false;
		}

		const std::string& modelSub = targetString.substr(first_ + 1);
		if (modelSub == "5_0")
		{
			outShaderModel = ERHIShaderModel::SM_5_0;
		}
		else
		{
			return false;
		}

		return true;
	}

	RHISwapChain::~RHISwapChain()
	{
		for (int i = 0; i < NumBackBuffers; ++i)
		{
			delete BackBufferResources[i];
			BackBufferResources[i] = nullptr;

			delete BackBufferRTVs[i];
			BackBufferRTVs[i] = nullptr;
		}
	}
	const ERHIDescriptorType RHIDescriptorHeap::GetType() const
	{
		return HeapType;
	}

	bool RHIDescriptorHeap::IsShaderVisible() const
	{
		return bIsShaderVisible;
	}
}

