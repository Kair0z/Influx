#include "InfluxGraphics/RHITexture.h"

#include "InfluxGraphics/RHIDevice.h"

namespace Influx::Graphics
{
	RHIRenderTargetView* RHITexture::GetAndOrCreateRenderTargetView(const RHIDevice* device)
	{
		if (mp_renderTargetView == nullptr)
		{
			mp_renderTargetView = device->CreateRenderTargetView((RHIResource*)GetResource());
		}
		
		return mp_renderTargetView;
	}

	RHIShaderResourceView* RHITexture::GetAndOrCreateShaderResourceView(const RHIDevice* device)
	{
		if (mp_shaderResourceView == nullptr)
		{
			mp_shaderResourceView = device->CreateShaderResourceView((RHIResource*)GetResource());
		}

		return mp_shaderResourceView;
	}

	RHIResource* RHITexture::GetResource() const
	{
		return mp_resource;
	}

	const RHITextureDesc& RHITexture::GetDesc() const
	{
		return m_desc;
	}

	const Math::Vectoru2& RHITexture::GetDimensions() const
	{
		return GetDesc().Dimensions;
	}

	const uint16 RHITexture::GetNumMips() const
	{
		return GetDesc().NumMips;
	}

	const ERHIFormat RHITexture::GetFormat() const
	{
		return GetDesc().Format;
	}
}