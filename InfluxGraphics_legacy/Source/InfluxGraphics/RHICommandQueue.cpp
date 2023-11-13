#include "InfluxGraphics/RHICommandQueue.h"

namespace Influx::Graphics
{
	RHICommandQueue::RHICommandQueue(const ERHICommandQueueType type)
		: m_type{type}
	{

	}

	const ERHICommandQueueType RHICommandQueue::GetType() const
	{
		return m_type;
	}
}

