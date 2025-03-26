#pragma once 
#include "core/math/colour.h"
#include "core/material/material.h"

namespace influx::renderer
{
	class mesh_material final
	{
	public:
		// shader slots: how to render
		// - vertex shader
		// - pixel shader

		// per-shader settings: what values to render
		// -

	private:
		material m_material{};
	};
}