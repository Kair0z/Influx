#pragma once

#ifndef __GR_RHI_TEXTURE_H_
#define __GR_RHI_TEXTURE_H_

#include "InfluxGraphics/RHITypes.h"

namespace influx::Graphics
{
	class RHIResource;
	class RHIRenderTargetView;
	class RHIShaderResourceView;
	class RHIDevice;

	struct RHITextureDesc final
	{
		Math::Vectoru2 Dimensions;
		uint16 NumMips;
		ERHIFormat Format;
	};

	class RHITexture final
	{
	private:
		RHITexture() = default;
		friend class RHIDevice;

		RHIResource* mp_resource;
		RHIRenderTargetView* mp_renderTargetView;
		RHIShaderResourceView* mp_shaderResourceView;

		RHITextureDesc m_desc;

	public:
		RHIRenderTargetView* GetAndOrCreateRenderTargetView(const RHIDevice* device);
		RHIShaderResourceView* GetAndOrCreateShaderResourceView(const RHIDevice* device);
		RHIResource* GetResource() const;

		const RHITextureDesc& GetDesc() const;
		const Math::Vectoru2& GetDimensions() const;
		const uint16 GetNumMips() const;
		const ERHIFormat GetFormat() const;

	public:
		RHITexture(const RHITexture&) = delete;
		RHITexture(RHITexture&&) = delete;
		RHITexture& operator=(const RHITexture&) = delete;
		RHITexture& operator=(RHITexture&&) = delete;
		virtual ~RHITexture() = default;
	};
}

#endif