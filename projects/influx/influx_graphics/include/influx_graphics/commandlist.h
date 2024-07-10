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
	class descriptor_heap;
	class input_resource_view;

	struct draw_instanced_args final
	{
		uint32 m_num_vertices_per_instance = 1u;
		uint32 m_num_instances = 1u;
		uint32 m_start_vertex = 0;
		uint32 m_start_instance = 0;
	};

	// draw_indexed_instanced!
	struct draw_indexed_args final
	{
		uint32 m_num_indexes_per_instance;
		uint32 m_num_instances;
		uint32 m_start_index;
		int m_start_vertex;
		uint32 m_start_instance;
	};

	struct copy_texture_args final
	{
		struct
		{
			uint32 m_subresource = 0u;
		} m_src;
		struct
		{
			uint32 m_subresource = 0u;
		} m_dest;
	};

	struct copy_buffer_args final
	{
		uint32 m_dest_offset = 0u;
		uint32 m_src_offset = 0u;
		
	};

	class command_list : public base
	{
	public:
		virtual void start(command_allocator* allocator, pipeline* init_state = nullptr) = 0;
		
		virtual void draw_instanced(const draw_instanced_args& args) = 0;
		
		virtual void draw_indexed(const draw_indexed_args& args) = 0;

		virtual void set_constants(uint32 param_index, uint32 num_dwords, void* source_data) = 0;

		virtual void set_indexbuffer(resource* index_buffer) = 0;
		
		virtual void set_vertexbuffer(resource* vertex_buffer) = 0;

		virtual void clear_rtv(render_target_view* view, const math::vectorf4& clear_value) = 0;

		virtual void transition_resource(resource* resource, e_resource_state before, e_resource_state after) = 0;

		virtual void copy_resource(resource* source, resource* dest) = 0;

		virtual void copy_texture(resource* src, resource* dest, const copy_texture_args& = {}) = 0;

		virtual void copy_buffer(resource* src, resource* dest, uint32 bytesize, const copy_buffer_args & = {}) = 0;

		virtual void set(descriptor_heap* heap) = 0;

		virtual void set(render_target_view* rtv) = 0;

		virtual void set(input_resource_view* srv, uint32 param_idx) = 0;

		virtual void set(rootsignature* rootsig) = 0;
		
		virtual void set(pipeline* pipeline) = 0;

		virtual void set(const viewport& viewport) = 0;

		virtual void set(const rect& rect) = 0;

		virtual void set(e_primitive_topology topo) = 0;

		virtual void end() = 0;
	};
}