#pragma once

#ifndef __GR_RHI_RESOURCE_H_
#define __GR_RHI_RESOURCE_H_

#include "Types.h"
#include "RHITypes.h"

namespace Influx::Graphics
{
	/* RHIResource */
	class RHIResource
	{
	public:
		ERHIResourceState GetCurrentState() const;
		ERHIResourceState GetPreviousState() const;

		void TransitionState(const ERHIResourceState newState);

	private:
		virtual void OnTransitionState(const ERHIResourceState before, const ERHIResourceState after) = 0;

		ERHIResourceState m_previousState;
		ERHIResourceState m_currentState;

	protected:
		RHIResource(ERHIResourceState initialState);

	public:
		RHIResource(const RHIResource&) = delete;
		RHIResource(RHIResource&&) = delete;
		RHIResource& operator=(const RHIResource&) = delete;
		RHIResource& operator=(RHIResource&&) = delete;
		virtual ~RHIResource() = default;
	};
}

#endif