#pragma once

#include "Runtime/RHI/RHITypes.h"
#include "Core/Container/Vector.h"
#include "Core/Math/Math.h"

namespace Influx
{
	class RenderAPI;
	class RHIRenderTarget
	{
	public:
		enum class ERenderTargetType { ColourTarget, DepthTarget };
		struct RenderTargetConfig
		{
			ERenderTargetType Type = ERenderTargetType::ColourTarget;
			Vector4f ClearValue = {0.0f,0.0f,0.0f,1.0f}; // In case of single channel, just uses R.
			uint32_t SampleQuality = 0;
			uint32_t SampleCount = 1;
			uint32_t ArraySize = 1;
			uint32_t MipLevels = 1;
			ERHIResourceFlags ResourceFlags = ERHIResourceFlags::AllowRenderTarget;
		};

		/* Resize & recreate resources */
		virtual void Resize(const Ptr<RenderAPI> api, const Vector2u& newSize) = 0;

		const Vector2u& GetDimensions() const;
		const RenderTargetConfig& GetConfig() const;
		const ERHIFormat GetFormat() const;

		virtual ~RHIRenderTarget() = default;

	protected:
		Vector2u mDimensions;
		ERHIFormat mFormat;
		ERenderTargetType mType;
		RenderTargetConfig mConfig;

		RHIRenderTarget(const Vector2u& dimensions, const ERHIFormat format, const ERenderTargetType type,
			const RenderTargetConfig& config = RenderTargetConfig())
			: mDimensions{ dimensions }, mFormat{ format }, mConfig{ config }, mType{ type }{};

		RHIRenderTarget(const RHIRenderTarget&) = delete;
		RHIRenderTarget(RHIRenderTarget&&) = delete;
		RHIRenderTarget& operator=(const RHIRenderTarget&) = delete;
		RHIRenderTarget& operator=(RHIRenderTarget&&) = delete;
	};

	class RHIRenderTargetView
	{
		
	};
}


