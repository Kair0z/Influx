#include "InfluxGraphics/RHIResource.h"

#include "InfluxGraphics/RHIDevice.h"

namespace Influx::Graphics
{
	RHIResource::RHIResource(ERHIResourceState initialState, const RHIClearValue& optimizedClearValue)
		: m_previousState{ initialState }
		, m_currentState{ initialState }
		, m_optimizedClearValue{ optimizedClearValue }
	{
	}

	void RHIResource::TransitionState(const ERHIResourceState newState)
	{
		m_previousState = GetCurrentState();
		m_currentState = newState;

		OnTransitionState(GetPreviousState(), GetCurrentState());
	}

	RHIResource::RenderTargetViewPtr RHIResource::CreateRenderTargetView(const DevicePtr device) const
	{
		return device->CreateRenderTargetView((RHIResource*)this);
	}

	RHIResource::ShaderResourceViewPtr RHIResource::CreateShaderResourceView(const DevicePtr device) const
	{
		return device->CreateShaderResourceView((RHIResource*)this);
	}

	ERHIResourceState RHIResource::GetCurrentState() const
	{
		return m_currentState;
	}

	ERHIResourceState RHIResource::GetPreviousState() const
	{
		return m_previousState;
	}

	const RHIClearValue& RHIResource::GetOptimizedClearValue() const
	{
		return m_optimizedClearValue;
	}
}

