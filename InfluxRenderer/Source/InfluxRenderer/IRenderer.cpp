#include "renderer_pch.h"
#include "InfluxRenderer/IRenderer.h"

namespace Influx::Renderer
{
	void IRenderer::Initialize(const DevicePtr device)
	{
		if (IsInitialized())
		{
			return;
		}

		OnInitialize(device);
		m_currentState.IsInitialized = true;
	}

	void IRenderer::Render(CommandListPtr commandList) const
	{
		if (!IsInitialized())
		{
			return;
		}

		OnRender(commandList);
	}

	void IRenderer::Cleanup(const DevicePtr device)
	{
		if (!IsInitialized())
		{
			return;
		}

		OnCleanup(device);
		m_currentState.IsInitialized = false;
	}

	void IRenderer::AttachToRenderTarget(const DevicePtr device, const RenderTargetPtr newRenderTarget)
	{
		if (!IsInitialized())
		{
			return;
		}

		OnAttachToRenderTarget(device, newRenderTarget);
		m_currentState.IsAttachedToRenderTarget = true;
	}

	void IRenderer::ResizeRenderTarget(const DevicePtr)
	{
		if (!IsInitialized())
		{
			return;
		}
	}

	void IRenderer::DetachFromRenderTarget(const DevicePtr device)
	{
		if (!IsInitialized())
		{
			return;
		}

		OnDetachFromRenderTarget(device);
		m_currentState.IsAttachedToRenderTarget = false;
	}

	bool IRenderer::IsInitialized() const
	{
		return m_currentState.IsInitialized;
	}

	bool IRenderer::IsAttachedToRenderTarget() const
	{
		return m_currentState.IsAttachedToRenderTarget;
	}
}