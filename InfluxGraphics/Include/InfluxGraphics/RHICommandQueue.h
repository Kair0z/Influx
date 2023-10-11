#pragma once

#ifndef __GR_RHI_COMMANDQUEUE_H_
#define __GR_RHI_COMMANDQUEUE_H_

#include "Types.h"
#include "RHITypes.h"

namespace influx::Graphics
{
	class RHICommandList;
	class RHIDevice;

	/* Command Queue */
	class RHICommandQueue
	{
	private:
		ERHICommandQueueType m_type;

	public:
		/* Serves a new RHICommandList to record commands. */
		virtual RHICommandList* SetupNewCommandList(RHIDevice* device) = 0;

		/* Executes a recorded RHICommandList */
		virtual void ExecuteCommmandList(RHICommandList* commandList) = 0;

		/* Flush all GPU work */
		virtual void Flush() = 0;

		const ERHICommandQueueType GetType() const;

	protected:
		RHICommandQueue(const ERHICommandQueueType type);

		RHICommandQueue(const RHICommandQueue&) = delete;
		RHICommandQueue(RHICommandQueue&&) = delete;
		RHICommandQueue& operator=(const RHICommandQueue&) = delete;
		RHICommandQueue& operator=(RHICommandQueue&&) = delete;
		virtual ~RHICommandQueue() = default;
	};
}

#endif