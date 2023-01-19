#pragma once

#include "RHIResourceView.h"

namespace Influx::Graphics
{
	class RHIRenderTargetView : public Internal::RHIResourceView<ERHIResourceViewType::RTV>
	{
	protected:
		RHIRenderTargetView(ERHIFormat rtvFormat);

	public:
		ERHIFormat GetFormat() const;

	private:
		ERHIFormat m_format;
	};
}


