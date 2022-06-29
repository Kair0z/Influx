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