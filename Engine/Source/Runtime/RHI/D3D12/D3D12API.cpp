#include "pch.h"
#include "D3D12API.h"
#include "D3D12Pipeline.h"
#include "D3D12RootSignature.h"
#include "D3D12SwapChain.h"
#include "D3D12CommandQueue.h"
#include "D3D12RenderTarget.h"
#include "Runtime/RHI/PipelineBuilder.h"

#include "Core/Type/Type.h"

namespace Influx
{
	void D3D12API::Initialize()
	{
		DxgiFactory = CreateDxgiFactory();
		DxgiAdapter = GetAdapter4(DxgiFactory, GetShouldUseWarp());
		DxDevice = D3D12API::CreateDevice(DxgiAdapter);
	}

	ID3D12Device2* D3D12API::GetDevice() const
	{
		return DxDevice;
	}

	/*Ptr<Buffer> D3D12API::CreateBuffer(const Buffer::Initializer& init)
	{
		return nullptr;
	}

	Ptr<Buffer> D3D12API::CreateVertexBuffer(const Buffer::Initializer& init)
	{
		return D3D12VertexBuffer::Create(init);
	}

	Ptr<Buffer> D3D12API::CreateIndexBuffer(const Buffer::Initializer& init)
	{
		return D3D12IndexBuffer::Create(init);
	}*/

	/*Ptr<Shader> D3D12API::CreateVertexShader(const String& filepath)
	{
		return Shader::Create(filepath, Shader::EType::VertexShader);
	}

	Ptr<Shader> D3D12API::CreatePixelShader(const String& filepath)
	{
		return Shader::Create(filepath, Shader::EType::PixelShader);
	}*/

	Ptr<RHIRenderTarget> D3D12API::CreateRenderTarget(const Vector2u& dimensions, const ERHIFormat format)
	{
		return D3D12RenderTarget::Create(this, dimensions, format, RHIRenderTarget::ERenderTargetType::ColourTarget);
	}

	Ptr<RHIRenderTarget> D3D12API::CreateDepthStencilTarget(const Vector2u& dimensions, const ERHIFormat format)
	{
		return D3D12RenderTarget::Create(this, dimensions, format, RHIRenderTarget::ERenderTargetType::DepthTarget);
	}

	Ptr<RHIGraphicsPipeline> D3D12API::CreateGraphicsPipeline(const GraphicsPipelineBuilder& desc)
	{
		/* Create 'a' Root Signature... */
		D3D12RootSignatureDesc rsDesc{};

		using namespace Internal;
		for (Internal::BaseResourceBinding* binding : desc.Bindings)
		{
			D3D12_SHADER_VISIBILITY vis{};
			switch (binding->GetShaderStageFlags())
			{
				case ERHIShaderStageFlags::Default: vis = D3D12_SHADER_VISIBILITY_ALL; break;
			}

			switch (binding->GetBindingType())
			{
			case ERHIResourceBindingType::Constants:
			{
				rsDesc.Parameters.push_back(D3D12RootParameter::AsConstants(binding->GetNum(), vis, 0, binding->GetBindingSpace()));
				break;
			}
				
			case ERHIResourceBindingType::CBV:
			{
				for (uint32_t i = 0; i < binding->GetNum(); ++i)
				{
					rsDesc.Parameters.push_back(D3D12RootParameter::AsCBV(vis, 0, binding->GetBindingSpace()));
				}
				break;
			} 

			case ERHIResourceBindingType::SRV:
			{
				for (uint32_t i = 0; i < binding->GetNum(); ++i)
				{
					rsDesc.Parameters.push_back(D3D12RootParameter::AsSRV(vis, 0, binding->GetBindingSpace()));
				}
				break;
			}

			case ERHIResourceBindingType::UAV:
			{
				for (uint32_t i = 0; i < binding->GetNum(); ++i)
				{
					rsDesc.Parameters.push_back(D3D12RootParameter::AsUAV(vis, 0, binding->GetBindingSpace()));
				}
				break;
			}
			}
		}
		/* No Static Sampler... Yet... */
		Ptr<D3D12RootSignature> signature = CreateRootSignature(rsDesc);

		/* Create Pipeline with Root Signature & Configured description... */
		return D3D12GraphicsPipeline::Create(this, signature, desc);
	}

	Ptr<RHISwapChain> D3D12API::CreateSwapChain(void* windowHandle, const Ptr<RHICommandQueue> commandQueue)
	{
		return D3D12SwapChain::Create(this, windowHandle, Cast<D3D12CommandQueue>(commandQueue)->GetD3D12CommandQueue());
	}

	Ptr<RHICommandQueue> D3D12API::CreateCommandQueue(const CommandQueueDesc& desc)
	{
		return D3D12CommandQueue::Create(this, desc);
	}

	void D3D12API::SetupDebugLayer()
	{
		D3D12API::EnableDebugLayer();
	}

	Ptr<D3D12RootSignature> D3D12API::CreateRootSignature(const D3D12RootSignatureDesc& desc)
	{
		return D3D12RootSignature::Create(this, desc);
	}

	D3D12API::~D3D12API()
	{
		SafeRelease(DxgiAdapter);
		SafeRelease(DxDevice);
		SafeRelease(DxgiFactory);
	}
}

