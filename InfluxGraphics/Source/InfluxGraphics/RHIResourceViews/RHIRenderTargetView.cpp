#include "InfluxGraphics/RHIResourceViews/RHIRenderTargetView.h"

namespace Influx::Graphics
{
    RHIRenderTargetView::RHIRenderTargetView(ERHIFormat rtvFormat)
        : m_format{rtvFormat}
    {
    }

    ERHIFormat RHIRenderTargetView::GetFormat() const
    {
        return m_format;
    }
}