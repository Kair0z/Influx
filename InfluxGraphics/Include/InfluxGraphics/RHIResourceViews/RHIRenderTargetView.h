#pragma once

#include "RHIResourceView.h"

#include "InfluxGraphics/RHIResource.h"

namespace Influx::Graphics
{
	class RHIRenderTargetView : public Internal::RHIResourceView<ERHIResourceViewType::RTV>
	{
	protected:
		RHIRenderTargetView(ERHIFormat rtvFormat, const RHIClearValue resourceClearValue);

	public:
		ERHIFormat GetFormat() const;
		const RHIClearValue& GetOptimizedClearValue() const;

	private:
		ERHIFormat m_format;
		const RHIClearValue m_resourceClearValue;
	};
}


