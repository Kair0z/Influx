#pragma once

#ifndef __GR_RHI_RESOURCE_H_
#define __GR_RHI_RESOURCE_H_

#include "Types.h"
#include "RHITypes.h"

#include "Core/Pointer.h"
#include "Core/Math/Vector.h"

namespace Influx::Graphics
{
	class RHIDevice;
	class RHIRenderTargetView;

	/* RHIResource */
	class RHIResource
	{
	protected:
		using DevicePtr = Ptr<RHIDevice>;
		using RenderTargetViewPtr = Ptr<RHIRenderTargetView>;

		RHIResource(ERHIResourceState initialState, const RHIClearValue& optimizedClearValue);

	public:
		void TransitionState(const ERHIResourceState newState);

		virtual RenderTargetViewPtr CreateRenderTargetView(const DevicePtr device) const = 0;

		ERHIResourceState GetCurrentState() const;
		ERHIResourceState GetPreviousState() const;
		const RHIClearValue& GetOptimizedClearValue() const;

	private:
		virtual void OnTransitionState(const ERHIResourceState before, const ERHIResourceState after) = 0;

		ERHIResourceState m_previousState;
		ERHIResourceState m_currentState;

		RHIClearValue m_optimizedClearValue;

	public:
		RHIResource(const RHIResource&) = delete;
		RHIResource(RHIResource&&) = delete;
		RHIResource& operator=(const RHIResource&) = delete;
		RHIResource& operator=(RHIResource&&) = delete;
		virtual ~RHIResource() = default;
	};
}

#endif