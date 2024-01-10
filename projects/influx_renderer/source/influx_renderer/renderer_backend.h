#pragma once
#include "influx_renderer.h"
#include "core/pointer.h"

namespace influx::renderer
{
	// backend singleton keeping static state for the renderer
	class renderer_backend final
		: public singleton<renderer_backend>
	{
	public:
		void initialize(const init_args& args);
		bool is_initialized() const;
		void cleanup();

	private:
		uni_ptr<class graphics_api> mp_graphics_api;
	};
}
