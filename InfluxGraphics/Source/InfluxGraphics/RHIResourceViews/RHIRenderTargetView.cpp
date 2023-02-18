#include "InfluxGraphics/RHIResourceViews/RHIRenderTargetView.h"

namespace Influx::Graphics
{
    RHIRenderTargetView::RHIRenderTargetView(ERHIFormat rtvFormat, const Math::Vectoru2& dimensions, const RHIClearValue resourceClearValue)
        : m_format{ rtvFormat }
        , m_resourceClearValue{ resourceClearValue }
        , m_resourceDimensions{dimensions}
    {
    }

    ERHIFormat RHIRenderTargetView::GetFormat() const
    {
        return m_format;
    }

    const RHIClearValue& RHIRenderTargetView::GetOptimizedClearValue() const
    {
        return m_resourceClearValue;
    }

    const Math::Vectoru2& RHIRenderTargetView::GetDimensions() const
    {
        return m_resourceDimensions;
    }
}