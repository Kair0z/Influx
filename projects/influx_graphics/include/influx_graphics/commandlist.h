#pragma once
#include "influx_graphics/base.h"
#include "core/math/vector.h"

#include "influx_graphics/resource.h"

namespace influx::graphics
{
	class command_allocator;
	class pipeline_state;
	class render_target_view;

	class command_list : public base
	{
	public:
		virtual void start(command_allocator* allocator, pipeline_state* init_state = nullptr) = 0;
		
		virtual void clear_rtv(render_target_view* view, const math::vectorf4& clear_value) = 0;

		virtual void transition_resource(resource* resource, e_resource_state new_state) = 0;

		virtual void end() = 0;
	};
}