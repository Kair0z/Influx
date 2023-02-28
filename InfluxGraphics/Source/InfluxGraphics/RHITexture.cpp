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
}