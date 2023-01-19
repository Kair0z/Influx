#include "InfluxGraphics/RHIDescriptorHeap.h"

namespace Influx::Graphics
{
	const ERHIResourceViewType RHIDescriptorHeap::GetType() const
	{
		return m_type;
	}

	const uint64 RHIDescriptorHeap::GetNumDescriptors() const
	{
		return m_numDescriptors;
	}

	bool RHIDescriptorHeap::IsShaderVisible() const
	{
		return m_isShaderVisible;
	}
}