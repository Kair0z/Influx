#include "Renderer.h"

namespace Influx
{
	IRenderer::IRenderer(RHIDevicePtr device)
		: mp_deviceRef{device}
	{
		Initialize(mp_deviceRef);
	}

	IRenderer::~IRenderer()
	{
		Cleanup(mp_deviceRef);
	}

	const IRenderer::RHIDevicePtr IRenderer::GetDeviceReference() const
	{
		return mp_deviceRef;
	}
}


