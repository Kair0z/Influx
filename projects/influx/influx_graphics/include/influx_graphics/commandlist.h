#pragma once
#include "influx_graphics/base.h"
#include "influx_graphics/resource.h"
#include "core/math/vector.h"

namespace influx::graphics
{
	class command_allocator;
	class pipeline;
	class render_target_view;
	class rootsignature;

	struct draw_instanced_args final
	{
		uint32 m_num_vertices_per_instance = 1u;
		uint32 m_num_instances = 1u;
		uint32 m_start_vertex = 0;
		uint32 m_start_instance = 0;
	};

	class command_list : public base
	{
	public:
		virtual void start(command_allocator* allocator, pipeline* init_state = nullptr) = 0;
		
		virtual void draw_instanced(const draw_instanced_args& args) = 0;

		virtual void clear_rtv(render_target_view* view, const math::vectorf4& clear_value) = 0;

		virtual void transition_resource(resource* resource, e_resource_state before, e_resource_state after) = 0;

		virtual void copy_resource(resource* source, resource* dest) = 0;

		virtual void set(render_target_view* rtv) = 0;

		virtual void set(rootsignature* rootsig) = 0;
		
		virtual void set(pipeline* pipeline) = 0;

		virtual void set(const viewport& viewport) = 0;

		virtual void set(const scissor_rect& rect) = 0;

		virtual void set(e_primitive_topology topo) = 0;

		virtual void end() = 0;
	};
}