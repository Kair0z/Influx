#pragma once

#include "RHIResourceView.h"

#include "InfluxGraphics/RHIResource.h"

namespace Influx::Graphics
{
	class RHIShaderResourceView : public Internal::RHIResourceView<ERHIResourceViewType::SRV>
	{
	protected:
		RHIShaderResourceView(ERHIFormat rtvFormat, const Math::Vectoru2& dimensions, const RHIClearValue resourceClearValue);

	public:
		ERHIFormat GetFormat() const;
		const RHIClearValue& GetOptimizedClearValue() const;
		const Math::Vectoru2& GetDimensions() const;

	private:
		ERHIFormat m_format;
		const RHIClearValue m_resourceClearValue;
		const Math::Vectoru2 m_resourceDimensions;
	};
}
