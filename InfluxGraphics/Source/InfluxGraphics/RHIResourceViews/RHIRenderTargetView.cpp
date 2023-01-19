#include "InfluxGraphics/RHIResourceViews/RHIRenderTargetView.h"

namespace Influx::Graphics
{
    RHIRenderTargetView::RHIRenderTargetView(ERHIFormat rtvFormat, const RHIClearValue resourceClearValue)
        : m_format{ rtvFormat }
        , m_resourceClearValue{ resourceClearValue }
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
}