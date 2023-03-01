#include "InfluxGraphics/D3D12/D3D12Device.h"

#include "InfluxGraphics/D3D12/D3D12CommandQueue.h"
#include "InfluxGraphics/D3D12/D3D12Swapchain.h"
#include "InfluxGraphics/D3D12/D3D12Resource.h"
#include "InfluxGraphics/D3D12/D3D12DescriptorHeap.h"
#include "InfluxGraphics/D3D12/D3D12PipelineLayout.h"
#include "InfluxGraphics/D3D12/D3D12Pipeline.h"

#include "InfluxGraphics/D3D12/ResourceViews/D3D12RenderTargetView.h"
#include "InfluxGraphics/D3D12/ResourceViews/D3D12ShaderResourceView.h"

namespace Influx::Graphics
{
	D3D12Device::D3D12Device(bool enableDebug) : RHIDevice()
	{
		if (enableDebug)
		{
			SetDebugLayerEnabled(true);
		}

		Initialize();
		PostInitialize();
	}

	D3D12Device::~D3D12Device()
	{
		PreCleanup();
		Cleanup();
	}

	void D3D12Device::Initialize()
	{
		mp_dxgiFactory = D3D12::CreateDxgiFactory4();
		mp_dxgiAdapter = D3D12::GetDxgiAdapter4(mp_dxgiFactory, true);
		mp_dxDevice = D3D12::CreateDxDevice2(mp_dxgiAdapter);

		// Cache DescriptorSizes
		m_cachedDsvDescriptorSize		= GetDxDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_cachedResourceDescriptorSize	= GetDxDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		m_cachedSamplerDescriptorSize	= GetDxDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		m_cachedRtvDescriptorSize		= GetDxDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	void D3D12Device::Cleanup()
	{
		Release(mp_dxDevice);
		Release(mp_dxgiAdapter);
		Release(mp_dxgiFactory);
	}

	RHICommandQueue* D3D12Device::CreateCommandQueue(const ERHICommandQueueType type) const
	{
		D3D12CommandQueue* result = new D3D12CommandQueue(type);

		result->mp_dxCommandQueue	= D3D12::CreateDxCommandQueue(GetDxDevice(), Conversion::ToDx12(type));
		result->mp_dxFence			= D3D12::CreateDxFence(GetDxDevice());

		return result;
	}

	RHIDevice::SwapchainPtr D3D12Device::CreateSwapchain(const Math::Vectoru2& dimensions, Platform::WindowHandle windowHandle, CommandQueuePtr commandQueue) const
	{
		D3D12Swapchain* result = new D3D12Swapchain(dimensions.x, dimensions.y, D3D12::CheckDxgiTearingSupport());
		D3D12CommandQueue* dxCommandQueue = static_cast<D3D12CommandQueue*>(commandQueue);

		result->m_renderTargetFormat = ERHIFormat::RGBA_8_Unorm;
		result->mp_dxgiSwapchain = D3D12::CreateDxgiSwapChain(mp_dxgiFactory, (::HWND)windowHandle, dxCommandQueue->GetDxCommandQueue(),
			dimensions.x, dimensions.y, RHISwapchain::GetNumBackBuffers(), Conversion::ToDx12(result->m_renderTargetFormat));

		result->m_currentBackBufferIndex = result->mp_dxgiSwapchain->GetCurrentBackBufferIndex();
		result->m_windowHandle = windowHandle;

		// Gather Backbuffer Resources & RTVs
		uint64 offsetSize = GetRTVDescriptorSize();
		for (uint8 i = 0; i < RHISwapchain::GetNumBackBuffers(); ++i)
		{
			// Get the buffer resources
			D3D12Resource* dxBufferResource = new D3D12Resource(ERHIResourceState::Present, RHIClearValue::Default());
			result->mp_dxgiSwapchain->GetBuffer(i, IID_PPV_ARGS(&dxBufferResource->mp_dxResource));
			result->mp_backBufferResources[i] = dxBufferResource;

			// Create the RenderTargetViews & store
			result->mp_backBufferRTVs[i] = CreateRenderTargetView(GetRTVDescriptorHeap(), dxBufferResource);
		}

		return result;
	}

	RHIDescriptorHeap* D3D12Device::CreateDescriptorHeap(const ERHIResourceViewType type, uint32 numDescriptors, bool isShaderVisible) const
	{
		D3D12DescriptorHeap* result = new D3D12DescriptorHeap(type, numDescriptors, isShaderVisible);

		D3D12_DESCRIPTOR_HEAP_FLAGS flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		result->mp_dxDescriptorHeap = D3D12::CreateDxDescriptorHeap(GetDxDevice(), Conversion::ToDx12(type), numDescriptors, flags);

		result->m_descriptorStride = GetDescriptorSize(type);

		return result;
	}

	RHIDevice::RenderTargetViewPtr D3D12Device::CreateRenderTargetView(const DescriptorHeapPtr descriptorHeap, const ResourcePtr viewedResource) const
	{
		D3D12Resource* d3d12Resource = (D3D12Resource*)viewedResource;
		D3D12DescriptorHeap* d3d12DescriptorHeap = (D3D12DescriptorHeap*)descriptorHeap;

		constexpr ERHIFormat temp_format = ERHIFormat::RGBA_8_Unorm;
		D3D12_RENDER_TARGET_VIEW_DESC desc{};
		desc.Format = Conversion::ToDx12(temp_format);

		const D3D12_RESOURCE_DESC& resource_desc = d3d12Resource->GetDxResource()->GetDesc();
		const Math::Vectoru2 resource_dimensions = { (uint32)resource_desc.Width, (uint32)resource_desc.Height };

		D3D12RenderTargetView* result = new D3D12RenderTargetView(temp_format, resource_dimensions, viewedResource->GetOptimizedClearValue());
		if (d3d12DescriptorHeap->GetHandles(result->m_dxCpuHandle, result->m_dxGpuHandle) != false)
		{
			GetDxDevice()->CreateRenderTargetView(d3d12Resource->GetDxResource(), nullptr, result->GetDxCPUHandle());
			return result;
		}

		return nullptr;
	}

	RHIDevice::ShaderResourceViewPtr D3D12Device::CreateShaderResourceView(const DescriptorHeapPtr descriptorHeap, const ResourcePtr viewedResource) const
	{
		D3D12Resource* d3d12Resource = (D3D12Resource*)viewedResource;
		D3D12DescriptorHeap* d3d12DescriptorHeap = (D3D12DescriptorHeap*)descriptorHeap;

		constexpr ERHIFormat temp_format = ERHIFormat::RGBA_8_Unorm;
		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Format = Conversion::ToDx12(temp_format);

		const D3D12_RESOURCE_DESC& resource_desc = d3d12Resource->GetDxResource()->GetDesc();
		const Math::Vectoru2 resource_dimensions = { (uint32)resource_desc.Width, (uint32)resource_desc.Height };

		D3D12ShaderResourceView* result = new D3D12ShaderResourceView(temp_format, resource_dimensions, viewedResource->GetOptimizedClearValue());
		if (d3d12DescriptorHeap->GetHandles(result->m_dxCpuHandle, result->m_dxGpuHandle) != false)
		{
			GetDxDevice()->CreateShaderResourceView(d3d12Resource->GetDxResource(), nullptr, result->GetDxCPUHandle());
			return result;
		}

		return nullptr;
	}

	RHIDevice::ResourcePtr D3D12Device::CreateResource(const ERHIResourceState initialState) const
	{
		D3D12Resource* result = new D3D12Resource(initialState, {});

		return result;
	}

	RHIDevice::ResourcePtr D3D12Device::CreateTextureResource(const ERHIResourceState initialState, const ERHIFormat format, const Math::Vectoru2& dimensions, const uint16 numMips) const
	{
		using namespace D3D12::HelperStructs;
		CommittedResourceDesc textureResourceDesc = CommittedResourceDesc::AsTexture(Conversion::ToDx12(format), dimensions.x, dimensions.y, numMips);

		RHIClearValue optimizedClearValue{};
		for (uint8 i = 0; i < 4u; ++i)
		{
			optimizedClearValue.Colour[i] = textureResourceDesc.GetOptimizedClearValue().Color[i];
		}
		
		D3D12Resource* result = new D3D12Resource(initialState, optimizedClearValue);

		result->mp_dxResource = D3D12::CreateDxCommittedResource(GetDxDevice(), textureResourceDesc, Conversion::ToDx12(initialState));

		return result;
	}

	RHIDevice::ResourcePtr D3D12Device::CreateVertexBufferResource(const ERHIResourceState initialState, const ERHIFormat format, const uint64 numBytesInBuffer) const
	{
		const bool useUploadHeap = true;
		const uint64 alignment = 0u;
		const RHIClearValue optimizedClearValue = {};

		using namespace D3D12::HelperStructs;
		CommittedResourceDesc bufferResourceDesc = CommittedResourceDesc::AsBuffer(useUploadHeap, numBytesInBuffer, alignment);

		D3D12Resource* result = new D3D12Resource(initialState, optimizedClearValue);
		result->mp_dxResource = D3D12::CreateDxCommittedResource(GetDxDevice(), bufferResourceDesc, Conversion::ToDx12(initialState));

		return result;
	}

	RHIDevice::ResourcePtr D3D12Device::CreateIndexBufferResource(const ERHIResourceState initialState, const ERHIFormat format, const uint64 numBytesInBuffer) const
	{
		const bool useUploadHeap = true;
		const uint64 alignment = 0u;
		const RHIClearValue optimizedClearValue = {};

		using namespace D3D12::HelperStructs;
		CommittedResourceDesc bufferResourceDesc = CommittedResourceDesc::AsBuffer(useUploadHeap, numBytesInBuffer, alignment);

		D3D12Resource* result = new D3D12Resource(initialState, optimizedClearValue);
		result->mp_dxResource = D3D12::CreateDxCommittedResource(GetDxDevice(), bufferResourceDesc, Conversion::ToDx12(initialState));

		return result;
	}

	RHIDevice::GraphicsPipelineLayoutPtr D3D12Device::CreateGraphicsPipelineLayout() const
	{
		D3D12GraphicsPipelineLayout* d3d12RootSignature = new D3D12GraphicsPipelineLayout();

		D3D12::HelperStructs::RootSignatureDesc rootSigDesc{};
		rootSigDesc.FeatureData;
		rootSigDesc.Flags;
		rootSigDesc.MaxVersion;
		rootSigDesc.StaticSamplers;
		
		d3d12RootSignature->mp_dxRootSignature = D3D12::CreateDxSerializedRootSignature(rootSigDesc, GetDxDevice());

		return d3d12RootSignature;
	}

	RHIDevice::GraphicsPipelinePtr D3D12Device::CreateGraphicsPipeline(const RHIGraphicsPipelineDescription& desc, GraphicsPipelineLayoutPtr rootSignature) const
	{
		D3D12GraphicsPipeline* d3d12Pipeline = new D3D12GraphicsPipeline(desc);
		D3D12GraphicsPipelineLayout* d3d12RootSignature = (D3D12GraphicsPipelineLayout*)rootSignature;

		D3D12::HelperStructs::GraphicsPipelineStateDesc pipelineDesc{};
		
		for (const RHIGraphicsPipelineDescription::InputElement& e : desc.InputElements)
		{
			pipelineDesc.AddInputElement(e.SemanticName.c_str(), e.SemanticIndex, Conversion::ToDx12(e.Format), e.InputSlot, e.AlignedByteOffset,
				e.bDataPerVertexNotPerInstance ? D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA : D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, e.InstanceDataStepRate);
		}

		pipelineDesc.VertexShaderBytecode.BytecodeLength	= desc.VS.size() * sizeof(byte);
		pipelineDesc.VertexShaderBytecode.pShaderBytecode	= desc.VS.data();
		pipelineDesc.PixelShaderByteCode.BytecodeLength		= desc.PS.size() * sizeof(byte);
		pipelineDesc.PixelShaderByteCode.pShaderBytecode	= desc.PS.data();
		pipelineDesc.DomainShaderByteCode.BytecodeLength	= desc.DS.size() * sizeof(byte);
		pipelineDesc.DomainShaderByteCode.pShaderBytecode	= desc.DS.data();
		pipelineDesc.HullShaderByteCode.BytecodeLength		= desc.HS.size() * sizeof(byte);
		pipelineDesc.HullShaderByteCode.pShaderBytecode		= desc.HS.data();
		pipelineDesc.GeometryShaderByteCode.BytecodeLength	= desc.GS.size() * sizeof(byte);
		pipelineDesc.GeometryShaderByteCode.pShaderBytecode	= desc.GS.data();

		pipelineDesc.PrimitiveTopologyType					= Conversion::ToDx12(desc.PrimitiveTopologyType);
		pipelineDesc.RasterizerState.AntialiasedLineEnable	= desc.RasterizerState.bEnableLineAA;
		pipelineDesc.RasterizerState.ConservativeRaster		= desc.RasterizerState.bEnableConservativeRaster ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		pipelineDesc.RasterizerState.CullMode				= Conversion::ToDx12(desc.RasterizerState.Cullmode);
		pipelineDesc.RasterizerState.DepthBias				= desc.RasterizerState.DepthBias;
		pipelineDesc.RasterizerState.DepthBiasClamp			= desc.RasterizerState.DepthBiasClamp;
		pipelineDesc.RasterizerState.DepthClipEnable		= desc.RasterizerState.bEnableDepthClip;
		pipelineDesc.RasterizerState.FillMode				= Conversion::ToDx12(desc.RasterizerState.Fillmode);
		pipelineDesc.RasterizerState.ForcedSampleCount		= desc.RasterizerState.ForcedSampleCount;
		pipelineDesc.RasterizerState.FrontCounterClockwise	= desc.RasterizerState.bFrontCounterClockwise;
		pipelineDesc.RasterizerState.MultisampleEnable		= desc.RasterizerState.bEnableMultisample;
		pipelineDesc.RasterizerState.SlopeScaledDepthBias	= desc.RasterizerState.SlopeScaledDepthBias;

		pipelineDesc.BlendState.AlphaToCoverageEnable = desc.BlendState.bEnableAlphaToCoverage;
		pipelineDesc.BlendState.IndependentBlendEnable = desc.BlendState.bEnableIndependentBlend;

		pipelineDesc.DepthStencilState.DepthEnable = desc.DepthStencilState.bEnableDepth;
		pipelineDesc.DepthStencilState.StencilEnable = desc.DepthStencilState.bEnableStencil;

		pipelineDesc.SampleDesc.Count	= desc.SampleCount;
		pipelineDesc.SampleDesc.Quality = desc.SampleQuality;
		pipelineDesc.SampleMask			= desc.SampleMask;

		for (uint8 i = 0; i < 8u; ++i)
		{
			if (desc.RenderTargets[i].Format != ERHIFormat::INVALID)
			{
				D3D12_RENDER_TARGET_BLEND_DESC& blendDesc = pipelineDesc.BlendState.RenderTarget[i];
				
				blendDesc.BlendEnable			= desc.RenderTargets[i].BlendDesc.bEnableBlend;
				blendDesc.LogicOpEnable			= desc.RenderTargets[i].BlendDesc.bEnableLogicOp;
				blendDesc.SrcBlend				= Conversion::ToDx12(desc.RenderTargets[i].BlendDesc.SrcBlend);
				blendDesc.DestBlend				= Conversion::ToDx12(desc.RenderTargets[i].BlendDesc.DestBlend);
				blendDesc.BlendOp				= Conversion::ToDx12(desc.RenderTargets[i].BlendDesc.BlendOperation);
				blendDesc.SrcBlendAlpha			= Conversion::ToDx12(desc.RenderTargets[i].BlendDesc.SrcBlendAlpha);
				blendDesc.DestBlendAlpha		= Conversion::ToDx12(desc.RenderTargets[i].BlendDesc.DestBlendAlpha);
				blendDesc.BlendOpAlpha			= Conversion::ToDx12(desc.RenderTargets[i].BlendDesc.BlendOperationAlpha);
				blendDesc.LogicOp				= Conversion::ToDx12(desc.RenderTargets[i].BlendDesc.LogicOperation);
				blendDesc.RenderTargetWriteMask = desc.RenderTargets[i].BlendDesc.RenderTargetWriteMask;

				pipelineDesc.RenderTargetFormats.push_back(Conversion::ToDx12(desc.RenderTargets[i].Format));
			}
		}

		d3d12Pipeline->mp_dxPipelineState = D3D12::CreateDxGraphicsPipelineState(pipelineDesc, d3d12RootSignature->mp_dxRootSignature, GetDxDevice());

		return d3d12Pipeline;
	}

	void D3D12Device::SetDebugLayerEnabled(bool setDebugLayerEnabled)
	{
		if (setDebugLayerEnabled)
		{
			D3D12::EnableDxDebugLayer();
		}
		else
		{
			D3D12::DisableDxDebugLayer();
		}
	}

	EGraphicsAPI D3D12Device::GetGraphicsAPI() const
	{
		return EGraphicsAPI::D3D12;
	}

	ID3D12Device2* D3D12Device::GetDxDevice() const
	{
		return mp_dxDevice;
	}

	IDXGIAdapter4* D3D12Device::GetDxgiAdapter() const
	{
		return mp_dxgiAdapter;
	}

	IDXGIFactory4* D3D12Device::GetDxgiFactory() const
	{
		return mp_dxgiFactory;
	}
}

