#pragma once

#ifndef __GR_RHI_RESOURCE_H_
#define __GR_RHI_RESOURCE_H_

#include "Types.h"
#include "RHITypes.h"

#include "Core/Pointer.h"

namespace Influx::Graphics
{
	class RHIDevice;
	class RHIRenderTargetView;

	/* RHIResource */
	class RHIResource
	{
	protected:
		RHIResource(ERHIResourceState initialState);
		using DevicePtr				= Ptr<RHIDevice>;
		using RenderTargetViewPtr	= Ptr<RHIRenderTargetView>;

	public:
		ERHIResourceState GetCurrentState() const;
		ERHIResourceState GetPreviousState() const;

		void TransitionState(const ERHIResourceState newState);

		virtual RenderTargetViewPtr CreateRenderTargetView(const DevicePtr device) const = 0;

	private:
		virtual void OnTransitionState(const ERHIResourceState before, const ERHIResourceState after) = 0;

		ERHIResourceState m_previousState;
		ERHIResourceState m_currentState;

	public:
		RHIResource(const RHIResource&) = delete;
		RHIResource(RHIResource&&) = delete;
		RHIResource& operator=(const RHIResource&) = delete;
		RHIResource& operator=(RHIResource&&) = delete;
		virtual ~RHIResource() = default;
	};
}

#endif