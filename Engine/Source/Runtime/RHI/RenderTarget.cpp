#include "pch.h"
#include "RenderTarget.h"

namespace Influx
{
    const Vector2u& RHIRenderTarget::GetDimensions() const
    {
        return mDimensions;
    }

    const RHIRenderTarget::RenderTargetConfig& RHIRenderTarget::GetConfig() const
    {
        return mConfig;
    }

    const ERHIFormat RHIRenderTarget::GetFormat() const
    {
        return mFormat;
    }
}

