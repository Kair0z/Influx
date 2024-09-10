#include "renderer_pch.h"
#include "influx_renderer/rendergraph/rendergraph.h"
#include "core/enum.h"

namespace influx::renderer
{
	void rendergraph::execute()
	{
		for (size_t layer_idx = 0u; layer_idx < m_layers.size(); ++layer_idx)
		{
			rglayer* layer = m_layers[layer_idx];

			// texture creates

			// buffer creates

			// texture transitions

			// buffer transitions
			
			// execute
			layer->execute();
			
			// texture destroys

			// buffer destroys
		}
	}

	void rgpool::tick()
	{
	}

	void rglayer::execute()
	{
		for (size_t pass_idx = 0u; pass_idx < m_passes.size(); ++pass_idx)
		{
			rgpass_base* pass = m_passes[pass_idx];


		}
	}
}

