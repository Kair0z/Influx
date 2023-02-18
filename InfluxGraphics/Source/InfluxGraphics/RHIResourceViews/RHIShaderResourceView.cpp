#include "InfluxGraphics/RHIResourceViews/RHIShaderResourceView.h"

namespace Influx::Graphics
{
    RHIShaderResourceView::RHIShaderResourceView(ERHIFormat rtvFormat, const Math::Vectoru2& dimensions, const RHIClearValue resourceClearValue)
        : m_format{ rtvFormat }
        , m_resourceClearValue{ resourceClearValue }
        , m_resourceDimensions{ dimensions }
    {
    }

    ERHIFormat RHIShaderResourceView::GetFormat() const
    {
        return m_format;
    }

    const RHIClearValue& RHIShaderResourceView::GetOptimizedClearValue() const
    {
        return m_resourceClearValue;
    }

    const Math::Vectoru2& RHIShaderResourceView::GetDimensions() const
    {
        return m_resourceDimensions;
    }
}