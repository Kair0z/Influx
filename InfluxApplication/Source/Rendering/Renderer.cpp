#include "Renderer.h"

namespace Influx
{
	IRenderer::IRenderer(RHIDevicePtr device)
		: mp_deviceRef{device}
	{
		
	}

	IRenderer::~IRenderer()
	{
		
	}

	const IRenderer::RHIDevicePtr IRenderer::GetDeviceReference() const
	{
		return mp_deviceRef;
	}
}


