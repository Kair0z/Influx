#pragma once

#ifndef __APP_IRENDERER_H_
#define __APP_IRENDERER_H_

#include "Common.h"
#include "InfluxGraphics/RHIDevice.h"

namespace Influx
{
	class IRenderer
	{
	protected:
		using RHIDevicePtr = SharedPtr<Graphics::RHIDevice>;
		RHIDevicePtr mp_deviceRef;

	public:
		virtual void OnRender() const = 0;

		IRenderer(const IRenderer&) = delete;
		IRenderer(IRenderer&&) = delete;
		IRenderer& operator=(const IRenderer&) = delete;
		IRenderer& operator=(IRenderer&&) = delete;

		const RHIDevicePtr GetDeviceReference() const;

	protected:
		IRenderer(RHIDevicePtr device);
		virtual ~IRenderer();

		virtual void Initialize(const RHIDevicePtr) = 0;
		virtual void Cleanup(const RHIDevicePtr) = 0;
	};
}

#endif

