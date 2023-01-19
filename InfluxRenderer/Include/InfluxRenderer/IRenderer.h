#pragma once

#include "Core/BasicTypes.h"
#include "Core/Pointer.h"

#include "RenderContext.h"

#include "Core/Container/List.h"
#include "Core/Platform/Platform.h"

#include "InfluxGraphics/RHITypes.h"

namespace Influx::Graphics
{
	class RHIDevice;
	class RHICommandList;
	class RHIRenderTargetView;
}

namespace Influx::Renderer
{
	class IRenderer
	{
	protected:
		using DevicePtr			= Ptr<Influx::Graphics::RHIDevice>;
		using CommandListPtr	= Ptr<Influx::Graphics::RHICommandList>;
		using RenderTargetPtr	= Ptr<Influx::Graphics::RHIRenderTargetView>;

	public:
		IRenderer() = default;

		/* Initializing RHI Resources */
		void Initialize(const DevicePtr device);

		/* Submitting work onto a passed RHICommandList */
		void Render(CommandListPtr commandList) const;

		/* Final cleaning up RHI Resources */
		void Cleanup(const DevicePtr);

		/* Attaching to a (new) render-target */
		void AttachToRenderTarget(const DevicePtr device, const RenderTargetPtr newRenderTarget);

		/* Signal resizing of the currently bound render-target */
		void ResizeRenderTarget(const DevicePtr);

		/* Detaching from currently bound render-target */
		void DetachFromRenderTarget(const DevicePtr);

		bool IsInitialized() const;
		bool IsAttachedToRenderTarget() const;

		const RenderTargetPtr GetCurrentRenderTarget() const;

	private:		
		virtual void OnInitialize(const DevicePtr) = 0;
		virtual void OnRender(const CommandListPtr) const = 0;
		virtual void OnCleanup(const DevicePtr) = 0;

		virtual void OnAttachToRenderTarget(const DevicePtr device, const RenderTargetPtr newRenderTarget) {};
		virtual void OnRenderTargetResize(const DevicePtr) {};
		virtual void OnDetachFromRenderTarget(const DevicePtr) {};

		struct State final
		{
			bool IsInitialized				= false;
			bool IsAttachedToRenderTarget	= false;
		};

		State m_currentState;
		RenderTargetPtr mp_currentlyBoundRenderTarget;

	public:
		IRenderer(const IRenderer&) = delete;
		IRenderer(IRenderer&&) = delete;
		IRenderer& operator=(const IRenderer&) = delete;
		IRenderer& operator=(IRenderer&&) = delete;
		virtual ~IRenderer() = default;
	};
}
