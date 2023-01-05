#pragma once

#ifndef __GR_RHI_RESOURCE_H_
#define __GR_RHI_RESOURCE_H_

#include "Types.h"
#include "RHITypes.h"

namespace Influx::Graphics
{
	/* Resource */
	class RHIResource
	{
		friend class RHIDevice;

	public:
		ERHIResourceState GetCurrentState() const;
		ERHIResourceState GetPreviousState() const;

		void TransitionState(const ERHIResourceState newState);

		virtual ~RHIResource() = default;

	protected:
		virtual void OnTransitionState(const ERHIResourceState before, const ERHIResourceState after) = 0;

		ERHIResourceState m_previousState;
		ERHIResourceState m_currentState;

		RHIResource(ERHIResourceState initialState);

		RHIResource(const RHIResource&) = delete;
		RHIResource(RHIResource&&) = delete;
		RHIResource& operator=(const RHIResource&) = delete;
		RHIResource& operator=(RHIResource&&) = delete;
	};

	class RHIRenderTargetView
	{
		friend class RHIDevice;

	public:
		
	protected:
		RHIRenderTargetView() = default;
		virtual ~RHIRenderTargetView() = default;

		RHIRenderTargetView(const RHIRenderTargetView&) = delete;
		RHIRenderTargetView(RHIRenderTargetView&&) = delete;
		RHIRenderTargetView& operator=(const RHIRenderTargetView&) = delete;
		RHIRenderTargetView& operator=(RHIRenderTargetView&&) = delete;
	};
}

#endif