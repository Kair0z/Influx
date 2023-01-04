#include "RHIResource.h"

namespace Influx::Graphics
{
	RHITexture::~RHITexture()
	{
		delete Resource;
		Resource = nullptr;

		delete RenderTargetView;
		RenderTargetView = nullptr;
	}

	RHITextureDescription::RHITextureDescription(uint32_t w, uint32_t h, ERHIFormat format, uint16_t numMips,
		const Math::Vector4f& optimizedClearValue, ERHIResourceState initialResourceState)
		: Width{w}, Height{h}, Format{format}, MipLevels{numMips}, OptimizedClearValue{optimizedClearValue}, InitialResourceState{initialResourceState}
	{

	}

	RHIVertexBuffer::~RHIVertexBuffer()
	{
		delete GpuResource;
		GpuResource = nullptr;
	}

	RHIConstantBuffer::~RHIConstantBuffer()
	{
		delete GpuResource;
		GpuResource = nullptr;

		delete ConstantBufferView;
		ConstantBufferView = nullptr;
	}
}