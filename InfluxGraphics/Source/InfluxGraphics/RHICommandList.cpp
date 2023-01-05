#include "InfluxGraphics/RHICommandList.h"

namespace Influx::Graphics
{
	RHICommandList::RHICommandList(const ERHICommandQueueType type)
		: m_type{ type }
	{

	}

	ERHICommandQueueType RHICommandList::GetType() const
	{
		return m_type;
	}
}

